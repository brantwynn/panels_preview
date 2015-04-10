<?php

/**
 * Content panes should not use default system/ajax. Use our own for now.
 */
function _panels_preview_add_path_to_ajax($element) {
  if (!empty($element['#ajax']) && !isset($element['#ajax']['path'])) {
    $element['#ajax']['path'] = 'system/panels-preview';
  }
  foreach (element_children($element) as $key) {
    _panels_preview_add_path_to_ajax($element[$key]);
  }
}

/**
 * Ajax callback that just returns the rendered preview.
 */
function panels_preview_ajax_update_preview($form, $form_state) {
  if (isset($form_state['values']) && isset($form['#panels_preview_preview_info']['configuration'])) {
    $form['#panels_preview_preview_info']['configuration'] = $form_state['values'] + $form['#panels_preview_preview_info']['configuration'];
  }
  return panels_preview_form_post_render_preview('', $form);
}

/**
 * Recursively parse form elements to add special autosubmit handling on a per field-type basis.
 */
function panels_preview_autosubmit_configure(&$element) {
  if (!empty($element['#type'])) {
    switch ($element['#type']) {
      case 'textfield':
        // Special handling for autosubmit.
        if (!empty($element['#autocomplete_path'])) {
          $element['#attributes']['class'][] = 'ctools-auto-submit-exclude panels-preview-autocomplete-autosubmit';
        }
        else {
          $element['#attributes']['class'][] = 'ctools-auto-submit-exclude panels-preview-textfield-autosubmit';
        }
        break;
      case 'text_format':
        $element['#attributes']['class'][] = 'ctools-auto-submit-exclude panels-preview-textarea-autosubmit';
        break;
      default:
        break;
    }
  }

  $children = element_children($element);

  if (!empty($children) && is_array($children)) {
    foreach ($children as $child) {
      panels_preview_autosubmit_configure($element[$child]);
    }
  }
}

/**
 * Implement the "Content Settings" fieldset in a pre-render. This fixes issues with image caused by initially
 * doing this in a form_alter.
 *
 * @see http://drupal.org/node/1567704
 */
function panels_preview_panelizer_pre_render($element) {
  $exclude_list = array(
    'form_id',
    'form_token',
    'form_build_id',
    'buttons',
  );

  // Add any remaining fields to the content settings fieldset.
  foreach (element_children($element) as $key) {
    $value = $element[$key];
    if (!in_array($key, $exclude_list) && !empty($value['#type']) && $value['#type'] != 'fieldset') {
      if (empty($element['content_settings'])) {
        // Add a content settings fieldset.
        $element['content_settings'] = array(
          '#type' => 'fieldset',
          '#title' => t('Content Settings'),
          '#weight' => 1,
        );
      }

      $element['content_settings'][$key] = $value;
      unset($element[$key]);
    }
  }

  return $element;
}

function panels_preview_form_ctools_entity_field_content_type_formatter_styles_alter(&$form, &$form_state, $form_id) {
  // Push the panels preview submitter onto the front of the submit array.
  // We need it at the beginning so the entity is saved before the IPE
  // reloads pane content.
  if (!empty($form_state['entity'])) {
    $form['#submit'][] = 'panels_preview_fape_submit';
  }
}

/**
 * Custom submit callback for FAPE enabled stylizer forms.
 */
function panels_preview_fape_submit(&$form, &$form_state) {
  // If there isn't a next step save the entity.
  if ((empty($form_state['triggering_element']['#next']) || !empty($form_state['field']['body'])) && !empty($form_state['entity']) && !empty($form_state['display_cache'])) {
    entity_save($form_state['entity_type'], $form_state['entity']);
    $form_state['display_cache']->display->context['panelizer']->data = $form_state['entity'];
  }
  // Otherwise we're on step 1, so just run the FAPE submitter.
  else {
    fape_field_edit_field_form_submit($form, $form_state);
  }
}

/**
 * Implementation of hook_form_FORM_ID_alter().
 *
 * Provides customization to panel pane style form
 */
function panels_preview_form_panels_edit_style_type_form_alter(&$form, &$form_state, $form_id) {

  if (!variable_get('panels_preview_show_panels_styles', 0)) {
    // Unset options for which we have no need
    unset($form['style']['#options']['block']);
    unset($form['style']['#options']['rounded_corners']);
    unset($form['style']['#options']['naked']);
  }

  // Better explain that "No Style" means "Default Pane Style"
  // Move the default option to the top. Also I should have learned
  // how to manage arrays in PHP.
  if (!empty($form['style']['#options']['default'])) {
    $form['style']['#options']['default'] = t('Default Pane Style');
    $default = $form['style']['#options']['default'];
    unset($form['style']['#options']['default']);
    $form['style']['#options'] = array_reverse($form['style']['#options']);
    $form['style']['#options']['default'] = $default;
    $form['style']['#options'] = array_reverse($form['style']['#options']);
  }

  // Better title for the List Style region style plugin
  if (!empty($form['style']['#options']['list'])) {
    $form['style']['#options']['list'] = t('List Style');
  }

  // Improve the UI around region style selections
  if (!empty($form['style']['#options'][0])) {
    $form['style']['#options'][0] = t('Default Region Style');
    if (empty($form['style']['#default_value']) || $form['style']['#default_value'] == '-1') {
      $form['style']['#default_value'] = '0';
    }
    unset($form['style']['#options']['default']);
  }

  // Move custom style option to bottom of list.
  if (!empty($form['style']['style']['#options']['stylizer'])) {
    unset($form['style']['style']['#options']['stylizer']);
    $form['style']['style']['#options']['stylizer'] = t('Custom Style');
  }

  // Adding a fieldset around styling
  $form['style'] = array('style' => $form['style']);
  $form['style']['#type'] = 'fieldset';
  $form['style']['#title'] = t('Style Settings');
}

/**
 * Implementation of hook_form_FORM_ID_alter().
 *
 * Provide customizations for the ctools stylizer edit form.
 */
function panels_preview_form_ctools_stylizer_edit_style_form_default_alter(&$form, &$form_state, $form_id) {
  // If Auto-submit is enabled.
  if (variable_get('panels_preview_live_preview', 1)) {

    // Customize the settings preview button.
    $form['top box']['preview']['submit']['#attributes'] = array(
      'class' => array('widget-preview-button', 'ctools-use-ajax', 'ctools-auto-submit-click'),
    );

    // Autosubmit the form.
    ctools_add_js('auto-submit');
    $form['#attributes']['class'][] = 'ctools-auto-submit-full-form';
    $form['#attributes']['class'][] = 'panels-preview-ctools-form';
    $form['top box']['preview']['#theme'] = 'panels_preview_stylizer_preview_form';

  }

  // Change the Weight Around
  $form['top box']['preview']['#weight'] = -50;
}

/**
 * Theme the stylizer preview form.
 */
function theme_panels_preview_stylizer_preview_form($vars) {
  $form = &$vars['form'];
  $plugin = $form['#form_state']['base_style_plugin'];
  $settings = $form['#form_state']['settings'];

  if (!empty($form['#form_state']['settings']['old_settings'])) {
    ctools_stylizer_cleanup_style($plugin, $form['#form_state']['settings']['old_settings']);
  }
  $preview = '';
  if (!empty($plugin['preview'])) {
    $preview = $plugin['preview'];
  }
  else {
    $base_types = ctools_get_style_base_types();

    if (!empty($base_types[$plugin['module']][$plugin['type']]['preview'])) {
      $preview = $base_types[$plugin['module']][$plugin['type']]['preview'];
    }
  }

  if (!empty($preview) && function_exists($preview)) {
    $output = '<fieldset id="preview" class="widget-preview-single"><legend>' . t('Preview') . '</legend><div class="fieldset-wrapper">';
    $output .= $preview($plugin, $settings);
    $output .= drupal_render_children($form);
    $output .= '</div></fieldset>';
    return $output;
  }
}

/**
 * Implementation of hook_form_FORM_ID_alter().
 *
 * Provide customizations for the ctools stylizer edit choose form.
 */
function panels_preview_form_ctools_stylizer_edit_style_form_choose_alter(&$form, &$form_state, $form_id) {

  // Remove the rounded corners options
  if (!empty($form['style_base']['Basic-styles']['pane_rounded_shadow'])) {
    unset($form['style_base']['Basic-styles']['pane_rounded_shadow']);
  }
  if (!empty($form['style_base']['Basic-styles']['region_rounded_shadow'])) {
    unset($form['style_base']['Basic-styles']['region_rounded_shadow']);
  }
}

/**
 *  Validator to ensure that reusable entites have titles.
 */
function panels_preview_reusable_entity_validate($element, &$form_state, $form) {
  // If the reusable entity checkbox is selected.
  if (!empty($form_state['values']['reusable'])) {
    // Ensure a title is present.
    if (empty($element['#value'])) {
      form_error($element, t('If you would like this entity to be reusable, please add a title.'));
    }
  }
}


/**
 * Implements hook_views_pre_view()
 */
function panels_preview_views_pre_view(&$view) {
  if (isset($view->display_handler->options['pane_conf'])) {
    $conf = $view->display_handler->options['pane_conf'];

    if (isset($conf['widget_title'])) {
      $view->display_handler->options['defaults']['title'] = FALSE;
      $view->display_handler->options['title'] = $conf['widget_title'];
      $view->build_info['title'] = $conf['widget_title'];
    }

    // Deal with legacy 'nodes' and others (such as 'files') view settings so
    // that other entity types can be included.
    if (!empty($conf['view_settings'])) {
      $conf['view_settings'] = panels_preview_convert_view_settings($conf['view_settings']);
    }

    // Set the style plugin to a table style.
    // Determine that this was previously a field view, which has been overridden to a node view in the pane config.
    if (!empty($conf['view_settings']) && $conf['view_settings'] == 'rendered_entity') {
      $view->display_handler->options['defaults']['row_plugin'] = FALSE;
      $view->display_handler->options['row_plugin'] = 'entity';
    }
    elseif (!empty($conf['view_settings']) && $conf['view_settings'] == 'table') {
      // Find the currently active field defination, else break out as table
      // needs fields.
      if (empty($view->display_handler->options['defaults']['fields']) && isset($view->display_handler->options['fields'])) {
        $fields = &$view->display_handler->options['fields'];
      }
      elseif (!empty($view->display_handler->default_display->options['fields'])) {
        $fields = &$view->display_handler->default_display->options['fields'];
      }
      else {
        // If no fields, don't try to display as table.
        return;
      }

      $view->display_handler->options['defaults']['style_plugin'] = FALSE;
      $view->display_handler->options['style_plugin'] = 'table';

      // Set or remove header labels depending on user selection.
      $use_header_titles = !empty($conf['header_type']) && $conf['header_type'] == 'titles';
      foreach ($fields as $field_key => &$field) {
        if ($use_header_titles && !empty($field['ui_name']) && empty($field['label'])) {
          $field['label'] = $field['ui_name'];
        }
        elseif (!$use_header_titles) {
          $field['label'] = '';
        }
        // Hide empty columns.
        if (!empty($view->display_handler->options['row_plugin']['hide_empty'])) {
          $view->display_handler->options['style_options'][$field_key]['empty_column'] = TRUE;
        }
      }
    }
    if ((empty($conf['view_settings']) || $conf['view_settings'] == 'rendered_entity') && !empty($conf['view_mode'])) {
      // Transfer over the row options from default if set to use.
      if (!empty($view->display_handler->options['defaults']['row_options'])) {
        $view->display_handler->options['defaults']['row_options'] = FALSE;
        $view->display_handler->options['row_options'] = $view->display_handler->default_display->options['row_options'];
      }
      $view->display_handler->options['row_options']['view_mode'] = $conf['view_mode'];
    }
  }
}

/**
 * Add the preview to the form output.
 *
 * It is done here so the form is fully processed.
 */
function panels_preview_form_post_render_preview($output, $form) {
  extract($form['#panels_preview_preview_info']);

  // If no preview type was specified, render the pane as normal.
  if (empty($preview_callback)) {
    $preview_callback = 'ctools_content_render';
  }
  $content = $preview_callback($pane->type, $pane->subtype, $configuration, $keywords, $args, $context);

  // Create the fieldset with appropriate content.
  $preview = array(
    '#type' => 'fieldset',
    '#title' => 'Preview',
    '#attributes' => array(
      'id' => 'panels-preview-form-widget-preview',
      'class' => array('widget-preview', 'widget-preview-single'),
    ),
    '#collapsible' => FALSE,
    '#weight' => -100,
  );
  if (!empty($content)) {
    $preview['preview']['#markup'] = (!empty($style['render pane'])) ? theme($style['render pane'], array('content' => $content, 'pane' => $pane, 'display' => $display, 'style' => $style, 'settings' => $pane->style['settings'])) : theme('panels_pane', array('content' => $content, 'pane' => $pane, 'display' => $display));
  }
  else {
    $preview['preview']['#markup'] = t('[no preview]');
  }
  return  drupal_render($preview) . $output;
}

/**
 * Convert with legacy 'nodes' and others (such as 'files') view settings.
 */
function panels_preview_convert_view_settings($view_settings) {
  // The 'fields' and 'table' view settings apply to any entity type.
  if (in_array($view_settings, array('fields', 'table'))) {
    return $view_settings;
  }

  // We convert other view settings to 'rendered_entity' (which could be
  // 'node' or 'files' or others that are specific to an entity type).
  return 'rendered_entity';
}

/**
 * Helper function to get the entity type retrieved by a view
 */
function panels_preview_get_entity_type($view) {
  $result = 'node';

  if (isset($view->base_table) && $view->base_table != 'node') {
    // Find the entity type with the corresponding base table.
    $base_table = $view->base_table;
    $entity_types = entity_get_info();
    foreach ($entity_types as $entity_type => $entity_type_info) {
      if ($entity_type_info['base table'] == $base_table) {
        $result = $entity_type;
        break;
      }
    }
  }

  return $result;
}

/**
 * Helper function to get view modes
 *
 * @param (optional) string $entity_type
 *   The type of entity for which to load view modes.
 */
function panels_preview_view_mode_options($entity_type = 'node') {
  $entity_info = entity_get_info($entity_type);
  $hidden_view_modes = array_filter(array_map('trim', explode("\n", variable_get('panels_preview_hidden_view_mode_options', PANELS_PREVIEW_HIDDEN_VIEW_MODE_OPTIONS))));

  $options = array();
  if (!empty($entity_info['view modes'])) {
    foreach ($entity_info['view modes'] as $mode => $settings) {
      if (!in_array($mode, $hidden_view_modes)) {
        $options[$mode] = $settings['label'];
      }
    }
  }
  return $options;
}

/**
 * Add the preview style selector to the form output.
 */
function panels_preview_form_preview_type_select() {
  $selector = array(
    '#type' => 'select',
    '#options' => array('split' => 'Split Preview', 'edit' => 'No Preview (form only)', 'full' => 'Full Preview (no form)'),
    '#attributes' => array(
      'id' => 'panels-preview-form-preview-select-type',
      'class' => array('widget-preview', 'widget-preview-select-type'),
    ),
  );
  return drupal_render($selector);
}

/**
 * Custom submit handler to save panels pane configuration for styling.
 */
function panels_preview_ctools_content_type_modal_submit(&$form, &$form_state) {
  $move = array('view_settings', 'header_type', 'view_mode', 'widget_title');
  foreach ($move as $key) {
    if (isset($form_state['values'][$key])) {
      $form_state['conf'][$key] = $form_state['values'][$key];
    }
  }
}
