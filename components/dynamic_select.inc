<?php

/**
 * Module specific instance of hook_help().
 */
function _webform_help_dynamic_select($section) {
  switch ($section) {
    case 'admin/settings/dynamic_select':
      return t('Creates dynamic select elements.');
  }
}

/**
 * @file
 *   Webform module dynamic select component.
 */

/**
 * Create a default dynamic select component.
 */
function _webform_defaults_dynamic_select() {
  return array(
    'name' => '',
    'form_key' => NULL,
    'email' => 1,
    'mandatory' => 0,
    'pid' => 0,
    'weight' => 0,
    'value' => '',
    'extra' => array(
      'items' => '',
      'options' => array(),
      'email' => 0,
      'multiple' => NULL,
      'aslist' => NULL,
      'description' => '',
    ),
  );
}

/**
 * This function contains the component edit form fields. 
 * Currently there are none.
 * @return
 *   An array of form items to be displayed on the edit component page.
 */
function _webform_edit_dynamic_select($currfield) {
  $edit_fields = array();
  $edit_fields['value'] = array(
    '#type' => 'textfield',
    '#title' => t('Default value'),
    '#default_value' => $currfield['value'],
    '#description' => t('Default value to select.'),
    '#size' => 60,
    '#maxlength' => 256,
    '#weight' => 0,
  );
  $edit_fields['extra']['options'] = array(
    '#type' => 'textarea',
    '#title' => t('Options'),
    '#default_value' => $currfield['extra']['options'],
    '#description' => t('PHP code for generating an options array. Must return an associative array.') .'<pre>$options = array(); ' . "\n".'$options["yes"] = "YES"; '."\n".'$options["no"] = "NO";'."\n".'return $options;</pre>',
    '#cols' => 60,
    '#rows' => 5,
    '#weight' => 1,
    '#required' => TRUE,
    '#validate' => array('_webform_edit_validate_dynamic_select' => array()),
  );
  $edit_fields['extra']['allowed_values'] = array(
    '#type' => 'textarea',
    '#title' => t('Allowed Values'),
    '#default_value' => $currfield['extra']['allowed_values'],
    '#description' => t('PHP code for generating an allowed values array. This field will overrided the options array for value validation. Code will follow the same format as the allowed options array. This field is useful if you populate your main options via ajax.'),
    '#cols' => 60,
    '#rows' => 5,
    '#weight' => 1,
    '#required' => FALSE,
    '#validate' => array('_webform_edit_validate_dynamic_select' => array()),
  );
  return $edit_fields;
}

/**
 * Element validation callback. 
 * TODO: Create validation rules
 */
function _webform_edit_validate_dynamic_select($element) {
  return TRUE;
}

/**
 * Build a form item array containing all the properties of this component.
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema.
 * @return
 *   An array of a form item to be displayed on the client-side webform.
 */
function _webform_render_dynamic_select($component) {
  $form_item = array(
    '#title'         => $component['name'],
    '#required'      => $component['mandatory'],
    '#weight'        => $component['weight'],
    '#description'   => _webform_filter_descriptions($component['extra']['description']),
    '#prefix'        => '<div class="webform-component-'. $component['type'] .'" id="webform-component-'. $component['form_key'] .'">',
    '#suffix'        => '</div>',
  );

  // Convert the user-entered options list into an array.
  $default_value = _webform_dynamic_select_filter_values($component['value']);
  $options = _webform_dynamic_select_options($component);

  // Set the component options.
  $form_item['#options'] = $options;

  // Set the default value.
  if ($default_value != '') {
    $form_item['#default_value'] = $default_value;
  }
  $form_item['#type'] = 'select';  
  return $form_item;
}

/**
 * Display the result of a textfield submission. The output of this function
 * will be displayed under the "results" tab then "submissions".
 * @param $data
 *   An array of information containing the submission result, directly
 *   correlating to the webform_submitted_data database schema.
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema.
 * @param $enabled
 *   If enabled, the value may be changed. Otherwise it will set to readonly.
 * @return
 *   Textual output formatted for human reading.
 */
function _webform_submission_display_dynamic_select($data, $component, $enabled = FALSE) {
  $form_item = _webform_render_dynamic_select($component);
  // Set the value as a single string.
  $form_item['#default_value'] = '';
  foreach ((array)$data['value'] as $value) {
    $form_item['#default_value'] = $value;
  }
  $form_item['#disabled'] = !$enabled;
  return $form_item;
}

/**
 * Format the output of emailed data for this component.
 *
 * @param $data
 *   A string or array of the submitted data.
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema.
 * @return
 *   Textual output to be included in the email.
 */
function theme_webform_mail_dynamic_select($data, $component) {
  // Convert submitted 'safe' values to un-edited, original form.
  $options = _webform_dynamic_select_options();

  // Generate the output.
  $output = '';
  if ($data !== '' && $options[$data]) {
    $output .= $component['name'] .': '. $options[$data] ."\n";
  }  
  return $output;
}

/**
 * Calculate and returns statistics about results for this component from all
 * submission to this webform. The output of this function will be displayed
 * under the "results" tab then "analysis".
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema
 * @return
 *   An array of data rows, each containing a statistic for this component's
 *   submissions.
 */
function _webform_analysis_rows_dynamic_select($component) {
  $options = _webform_dynamic_select_options();
  $query = 'SELECT data, count(data) as datacount '.
    ' FROM {webform_submitted_data} '.
    ' WHERE nid = %d '.
    ' AND cid = %d '.
    " AND data != '0' AND data != '' ".
    ' GROUP BY data ';
  $result = db_query($query, $component['nid'], $component['cid']);
  $rows = array();
  while ($data = db_fetch_array($result)) {
    if (isset($options[$data['data']])) {
      $display_option = $options[$data['data']];
    }
    else {
      $display_option = $data['data'];
    }
    $rows[] = array($display_option, $data['datacount']);
  }
  return $rows;
}

/**
 * Return the result of this component's submission for display in a table. The
 * output of this function will be displayed under the "results" tab then "table".
 * @param $data
 *   An array of information containing the submission result, directly
 *   correlating to the webform_submitted_data database schema
 * @return
 *   Textual output formatted for human reading.
 */
function _webform_table_data_dynamic_select($data) {
  $output = '';
  // Set the value as a single string.
  if (is_array($data['value'])) {
    foreach ($data['value'] as $value) {
      if ($value !== '0') {
        $output .= check_plain($value) .'<br />';
      }
    }
  }
  else {
    $output .= check_plain(empty($data['value']['0']) ? '' : $data['value']['0']);
  }
  return $output;
}

/**
 * Return the header information for this component to be displayed in a comma
 * seperated value file. The output of this function will be displayed under the
 * "results" tab then "download".
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema.
 * @return
 *   An array of data to be displayed in the first three rows of a CSV file, not
 *   including either prefixed or trailing commas.
 */
function _webform_csv_headers_dynamic_select($component) {
  $headers = array(
    0 => array(),
    1 => array(),
    2 => array(),
  );
  $headers[0][] = '';
  $headers[1][] = '';
  $headers[2][] = $component['name'];
  return $headers;
}

/**
 * Return the result of a field submission. The output of this function will
 * be displayed under the "results" tab then "submissions".
 * @param $data
 *   An array of information containing the submission result, directly
 *   correlating to the webform_submitted_data database schema.
 * @return
 *   Textual output formatted for CSV, not including either prefixed or trailing
 *   commas.
 */
function _webform_csv_data_dynamic_select($data, $component) {
  return $data['value'][0];
}

/**
  * Utility function to filter default values
  * @param $value
  *   Default value to filter
  * @return
  *   String representing value
**/
function _webform_dynamic_select_filter_values($value) {
  //TODO: ensure it is a member of the options array
  return check_plain($value);
}



/**
  * Utility function for retrieving data in array
  * It uses the same eval() code found in cck. 
  * TODO: Find if there is a safer way than eval;
  * $option[$key] => $value
  * @param $component
  *   An array representing the form component
  * @return
  *   An associative array of select options
**/
function _webform_dynamic_select_options($component) {
  // Load array from options
  ob_start();
    $options = eval($component['extra']['options']);
  ob_end_clean();
  return (is_array($options)) ? $options : array();
}