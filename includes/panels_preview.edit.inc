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
 * Implementation of hook_form_FORM_ID_alter()
 *
 * Provides customizations to the views content type settings form
 */
function panels_preview_form_views_content_views_panes_content_type_edit_form_alter(&$form, &$form_state, $form_id) {

  // Get the configuration
  $conf = $form_state['conf'];

  // Add a widget title setting if views allows this to be customized
  if (!empty($form['override_title'])) {
    $form['override_title_markup']['#access'] = FALSE;
    $form['override_title']['#access'] = FALSE;
    $form['override_title_text']['#access'] = FALSE;
    $form['override_title_heading']['#access'] = FALSE;
    $form['exposed']['widget_title'] = array(
      '#field_prefix' => t('Title'),
      '#type' => 'textfield',
      '#size' => '',
      '#weight' => -100,
      '#states' => array(
        'visible' => array(
          ':input[name="override_title"]' => array('value' => '1'),
        ),
      ),
      '#default_value' => (isset($conf['widget_title'])) ? $conf['widget_title'] : $form_state['view']->display_handler->options['title'],
      '#parents' => array('widget_title'),
    );
  }

  // Add an option to make the widget title a link if views allows this to happen
  if (!empty($form['link_to_view']) && !empty($form['path'])) {
    $form['link_to_view']['#title'] = t('Make title a link');
    $form['link_to_view']['#description'] =  '';
    $form['link_to_view']['#id'] = 'edit-link';
    $form['exposed']['link_to_view'] = $form['link_to_view'];
    $form['exposed']['link_to_view']['#parents'] = array('link_to_view');
    unset($form['link_to_view']);
  }

  // Add an option to make the more link available if views allows this to happen
  if (!empty($form['more_link'])) {
    $form['more_link']['#id'] = 'more-link';
    $form['more_link']['#description'] = '';
    $form['exposed']['more_link'] = $form['more_link'];
    $form['exposed']['more_link']['#parents'] = array('more_link');
    unset($form['more_link']);
  }


  // Update the field settings for pagers
  if (!empty($form['use_pager']) && !empty($form['pager_id'])) {
    $form['use_pager']['#prefix'] = '<div class="form-item container-inline">';
    $form['exposed']['use_pager'] = $form['use_pager'];
    $form['exposed']['use_pager']['#parents'] = array('use_pager');
    $form['exposed']['pager_id'] = $form['pager_id'];
    $form['exposed']['pager_id']['#parents'] = array('pager_id');
    unset($form['use_pager']);
    unset($form['pager_id']);
  }

  // Handle the path value that views gives us
  if (!empty($form['path'])) {
    $form['path']['#description'] = t('The URL path used for linking and paging purposes. Leave blank to use the current page.');
    $form['path']['#title'] = '';
    $form['path']['#dependency'] = array('edit-link' => array(1), 'more-link' => array(1), 'use-pager-checkbox' => array(1));
    $form['exposed']['path'] = $form['path'];
    $form['exposed']['path']['#parents'] = array('path');
    unset($form['path']);
  }

  // Adjust the items per page and offset settings
  if (!empty($form['items_per_page']) && !empty($form['offset'])) {
    $form['items_per_page']['#field_prefix'] = t('Items to Show');
    $form['items_per_page']['#title'] = '';
    $form['items_per_page']['#description'] = t('Enter 0 to display all.');
    $form['display_settings']['items_per_page'] = $form['items_per_page'];
    unset($form['items_per_page']);
    $form['offset']['#field_prefix'] = t('Items to Skip');
    $form['offset']['#title'] = '';
    $form['offset']['#description'] = t('Enter 0 to skip none.');
    $form['offset']['#prefix'] = '<div class="clearfix">';
    $form['offset']['#suffix'] = '</div>';
    $form['display_settings']['offset'] = $form['offset'];
    unset($form['offset']);
  }
  elseif (!empty($form['items_per_page']) && empty($form['offset'])) {
    $form['items_per_page']['#field_prefix'] = t('Items to Show');
    $form['items_per_page']['#title'] = '';
    $form['items_per_page']['#description'] = t('Enter 0 to display all.');
    $form['items_per_page']['#prefix'] = '<div class="clearfix">';
    $form['items_per_page']['#suffix'] = '</div>';
    $form['display_settings']['items_per_page'] = $form['items_per_page'];
    unset($form['items_per_page']);
  }
  elseif (empty($form['items_per_page']) && !empty($form['offset'])) {
    $form['offset']['#field_prefix'] = t('Items to Skip');
    $form['offset']['#title'] = '';
    $form['offset']['#description'] = t('Enter 0 to skip none.');
    $form['offset']['#prefix'] = '<div class="clearfix">';
    $form['offset']['#suffix'] = '</div>';
    $form['display_settings']['offset'] = $form['offset'];
    unset($form['offset']);
  }

  // Adjust the sort ordering and sort by options
  if (!empty($form['exposed']['sort_order']) && !empty($form['exposed']['sort_by'])) {
    $form['exposed']['sort_order']['#title'] = t('Sort order');
    $form['display_settings']['sort_order'] = $form['exposed']['sort_order'];
    $form['display_settings']['sort_order']['#parents'] = array('exposed', 'sort_order');
    unset($form['exposed']['sort_order']);
    $form['exposed']['sort_by']['#prefix'] = '<div class="clearfix">';
    $form['exposed']['sort_by']['#suffix'] = '</div>';
    $form['display_settings']['sort_by'] = $form['exposed']['sort_by'];
    $form['display_settings']['sort_by']['#parents'] = array('exposed', 'sort_by');
    unset($form['exposed']['sort_by']);
  }
  elseif (!empty($form['exposed']['sort_order']) && empty($form['exposed']['sort_by'])) {
    $form['exposed']['sort_order']['#title'] = t('Sort order');
    $form['exposed']['#prefix'] = '<div class="clearfix">';
    $form['exposed']['#suffix'] = '</div>';
    $form['display_settings']['sort_order'] = $form['exposed']['sort_order'];
    $form['display_settings']['sort_order']['#parents'] = array('exposed', 'sort_order');
    unset($form['exposed']['sort_order']);
  }
  elseif (empty($form['exposed']['sort_order']) && !empty($form['exposed']['sort_by'])) {
    $form['exposed']['sort_by']['#prefix'] = '<div class="clearfix">';
    $form['exposed']['sort_by']['#suffix'] = '</div>';
    $form['display_settings']['sort_by'] = $form['exposed']['sort_by'];
    $form['display_settings']['sort_by']['#parents'] = array('exposed', 'sort_by');
    unset($form['exposed']['sort_by']);
  }

  // Adjust the field setting options
  if (isset($form['fields_override'])) {
    $form['fields_override']['#title'] = t('Field Settings');
    $form['fields_override']['#collapsible'] = FALSE;
    foreach ($form['fields_override'] as &$field) {
      if (is_array($field)) {
        $field['#title'] = t('Display') . ' ' . $field['#title'];
      }
    }
  }

  // Determine if this is a fielded view. If so, add magic display type changer
  $view_handler = $form_state['view']->display_handler;
  if ($view_handler->options['row_plugin'] == 'fields') {
    // Set to default view settings if there isn't one.
    if (empty($conf['view_settings'])) {
      if ($form_state['view']->style_plugin->plugin_name == 'table') {
        $conf['view_settings'] = 'table';
      }
      else {
        $conf['view_settings'] = 'fields';
      }
    }

    // Deal with legacy 'nodes' and others (such as 'files') view settings so
    // that other entity types can be included.
    $conf['view_settings'] = panels_preview_convert_view_settings($conf['view_settings']);

    // Add information about the View Mode
    $form['display_settings']['view_settings'] = array(
      '#type' => 'radios',
      '#prefix' => '<div class="view-settings-wrapper"><span class="field-prefix">' . t('Display Type') . '</span>',
      '#suffix' => '</div>',
      '#default_value' => $conf['view_settings'],
      '#weight' => 10,
      '#options' => array(
        'fields' => t('Fields'),
        'rendered_entity' => t('Content'),
        'table' => t('Table'),
      ),
    );

    // Add header column options for table views.
    $form['display_settings']['header_type'] = array(
      '#type' => 'select',
      '#title' => t('Column Header'),
      '#options' => array(
        'none' => t('None'),
        'titles' => t('Titles'),
      ),
      '#default_value' => !empty($conf['header_type']) ? $conf['header_type'] : 'none',
      '#states' => array(
        'visible' => array(
          ':input[name="view_settings"]' => array('value' => 'table'),
        ),
      ),
      '#weight' => 11,
    );

    // Update the field overrides to be dependent on the view settings selection.
    $form['fields_override']['#states'] = array(
      // The inverted logic here isn't optimal, and in the future may be better achieved via OR'd conditions.
      // @link http://drupal.org/node/735528 @endlink
      'invisible' => array(
        ':input[name="view_settings"]' => array('value' => 'rendered_entity'),
      ),
    );
  }

  // Get view modes for entity
  $view_modes = panels_preview_view_mode_options(panels_preview_get_entity_type($form_state['view']));

  // Get the default view mode.
  $options_default_view_mode = ($view_handler->options['row_plugin'] == 'fields') ? 'teaser' : 'full';
  if (isset($form_state['view']->style_plugin->display->display_options['row_options']['view_mode'])) {
    $options_default_view_mode = $form_state['view']->style_plugin->display->display_options['row_options']['view_mode'];
  }
  if (!array_key_exists($options_default_view_mode, $view_modes)){
    $options_default_view_mode = current(array_keys($view_modes));
  }

  // Add specific style options.
  $form['content_settings']['view_mode'] = array(
    '#type' => 'radios',
    '#options' => $view_modes,
    '#default_value' => !empty($conf['view_mode']) ? $conf['view_mode'] : $options_default_view_mode,
    '#states' => array(
      'visible' => array(
        ':input[name="view_settings"]' => array('value' => 'rendered_entity'),
      ),
    ),
  );

  // Define a general settings fieldset if we have exposed values
  if (!empty($form['exposed']) && count(element_children($form['exposed'])) > 0) {
    $form['exposed']['#type'] = 'fieldset';
    $form['exposed']['#title'] = t('General Settings');
    $form['exposed']['#weight'] = -30;
    $form['exposed']['#attributes'] = array('class' => array('general-settings-fieldset'));
  }

  // Define a context settings fieldset if we have exposed values
  if (!empty($form['context']) && count(element_children($form['context'])) > 0) {
    $form['context']['#type'] = 'fieldset';
    $form['context']['#title'] = t('Context Settings');
    $form['context']['#weight'] = -29;
  }

  // Define a content settings fieldset if we have exposed values
  if (!empty($form['content_settings']) && count(element_children($form['content_settings'])) > 0) {
    $form['content_settings']['#type'] = 'fieldset';
    $form['content_settings']['#title'] = t('Content Settings');
    $form['content_settings']['#weight'] = -27;
  }

  // Define a display settings fieldset if we have display options
  if (!empty($form['display_settings']) && count(element_children($form['display_settings'])) > 0) {
    $form['display_settings']['#type'] = 'fieldset';
    $form['display_settings']['#title'] = t('Display Settings');
    $form['display_settings']['#weight'] = -28;
  }

  // Add auto submit functionality
  if (variable_get('panels_preview_live_preview', 1)) {
    panels_preview_autosubmit_configure($form);
  }

  // Add a custom submit handler to our preview and submit option
  $form['#submit'][] = 'panels_preview_views_content_type_modal_submit';
}

/**
 * Implementation of hook_form_FORM_ID_alter()
 *
 * Provides customization to entity formatter option form
 */
function panels_preview_form_ctools_entity_field_content_type_formatter_options_alter(&$form, &$form_state, $form_id) {

  // Get the configuration
  $conf = $form_state['conf'];

  // Move the title for the label and formatter to be the field prefix
  foreach (array('label', 'formatter') as $field) {
    if (!empty($form['general_settings'][$field])) {
      $form['general_settings'][$field]['#field_prefix'] = $form['general_settings'][$field]['#title'];
      $form['general_settings'][$field]['#title'] = '';
    }
  }

  // Add in the field edit fields with FAPE if we are not editing a default
  if (empty($form_state['entity']) && !empty($form_state['display']->context['panelizer']->data)) {
    // Determine what entity type and field name we are editing based on the form subtype and URL args.
    $arg_parts = explode(':', arg(4));
    list($entity_type, $field_name) = explode(':', $form_state['subtype_name']);
    $entity = reset(entity_load($entity_type, array($arg_parts[2])));

    // Unfortunately the above code is not as reliable as we'd like. There are cases where the field we're editing
    // doesn't have an corrosponding entity (say, comments). Let's make sure we have an entity before adding FAPE specific
    // data into the form.
    if (!empty($entity)) {
      $form_state['entity'] = $entity;
      $form_state['entity_type'] = $entity_type;
      $form_state['field_name'] = $field_name;
      $form_state['bundle'] = empty($entity->bundle) ? $entity->type : $entity->bundle;
      $form_state['field_instance'] = field_info_instance($entity_type, $field_name, $form_state['bundle']);
      $form_state['langcode'] = LANGUAGE_NONE;
      $form_state['subform_id'] = 'fape_field_edit_field_form';
    }
  }

  // Ensure form id, form token, and form build id are on the root of the form.
  // Image fields in particular have issues with this.
  $form['form_id'] = $form['general_settings']['form_id'];
  $form['form_token'] = $form['general_settings']['form_token'];
  $form['form_build_id'] = $form['general_settings']['form_build_id'];
  unset($form['general_settings']['form_id']);
  unset($form['general_settings']['form_token']);
  unset($form['general_settings']['form_build_id']);

  if (!empty($form_state['entity'])) {
    // Add the field edit form.
    fape_field_edit_field_form($form, $form_state);

    // Remove the default submitter in favor of our own custom submit callback.
    array_pop($form['#submit']);
    $form['#submit'][] = 'panels_preview_fape_submit';
  }
  // Pre-rende fields into a fieldset.
  $form['#pre_render'][] = 'panels_preview_panelizer_pre_render';

  // When moving backwards through form steps, ensure image widgets properly retain their value.
  if (!empty($form_state['field_instance']) && $form_state['triggering_element']['#value'] == 'Back') {
    $field_name = $form_state['field_instance']['field_name'];
    $field_type = $form_state['field_instance']['widget']['module'];
    $entity = $form_state['entity'];

    // Reset the #default_value to the (currently) unsaved entity value.
    if ($field_type == 'image' && !empty($form[$field_name])) {
      $field = $entity->$field_name;
      foreach ($field[LANGUAGE_NONE] as $delta => $value) {
        $form[$field_name][LANGUAGE_NONE][$delta]['#default_value'] = $value;
      }
    }
  }

  // Enable auto submit functionality
  if (variable_get('panels_preview_live_preview', 1)) {
    panels_preview_autosubmit_configure($form);
  }

  // Add a custom submit handler to our preview and submit option
  $form['#submit'][] = 'panels_preview_views_content_type_modal_submit';
  $form['buttons']['preview']['#submit'][] = 'panels_preview_views_content_type_modal_submit';
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
 * Custom submit handler to save panels pane configuration for styling
 */
function panels_preview_views_content_type_modal_submit(&$form, &$form_state) {
  $move = array('view_settings', 'header_type', 'view_mode', 'widget_title');
  foreach ($move as $key) {
    if (isset($form_state['values'][$key])) {
      $form_state['conf'][$key] = $form_state['values'][$key];
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
