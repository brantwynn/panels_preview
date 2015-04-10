<?php

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
