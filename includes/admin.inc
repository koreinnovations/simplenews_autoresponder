<?php

// Id

/**
 * @file
 * Administration pages for simplenews autoresponder.
 */

/**
 * Administration pages form callback.
 */
function simplenews_autoresponder_admin() {
  $form = array();
  $form['add_link'] = array(
      '#value' => theme_item_list(array(
          l('Create Simplenews autoresponder', 'admin/settings/simplenews_autoresponder/add'),
      )),
  );

  $header = array(
      t('Autoresponder name'),
      t('Actions'),
  );

  $items = simplenews_get_active_autoresponders();
  $rows = array();

  foreach ($items as $item) {
    $links = array();
    $links[] = l(t('Edit'), 'admin/settings/simplenews_autoresponder/edit/' . $item->id);
    $links[] = l(t('Delete'), 'admin/settings/simplenews_autoresponder/delete/' . $item->id);

    $rows[] = array(
        check_plain($item->name),
        implode(' &nbsp;|&nbsp; ', $links),
    );
  }

  $form['table'] = array(
      '#value' => theme('table', $header, $rows),
  );


  return $form;
}

/**
 * Form callback for simplenews autoresponder edit form.
 */
function simplenews_autoresponder_admin_edit(&$form_state, $id = -1) {
  // Go back if cancelled
  if (isset($form_state['post']) && isset($form_state['post']['cancel'])) {
    drupal_goto('admin/settings/simplenews_autoresponder');
  }

  // Load old data if editing existing entry.
  $old_data = array();
  if ($id > -1) {
    $old_data = simplenews_autoresponder_get_item($id);
  }

  // Build form.
  $form['name'] = array(
      '#type' => 'textfield',
      '#title' => t("Autoresponder name"),
      '#default_value' => isset($old_data['name']) ? check_plain($old_data['name']) : '',
      '#required' => TRUE,
  );

  $form['enabled'] = array(
      '#type' => 'radios',
      '#title' => t('Enable this autoresponder'),
      '#default_value' => isset($old_data['enabled']) ? check_plain($old_data['enabled']) : 0,
      '#description' => t("When enabled, autoresponders will be sent. When disabled, nothing will happen."),
      '#options' => array(t('Disabled'), t('Enabled')),
  );

  $form['tid'] = array(
      '#type' => 'select',
      '#title' => t('Newsletter'),
      '#default_value' => isset($old_data['tid']) ? $old_data['tid'] : 0,
      '#options' => simplenews_autoresponder_get_tids(),
      '#description' => t("Choose a newsletter as a source for your autoresponder emails."),
      '#required' => TRUE,
  );

  $form['sort'] = array(
      '#type' => 'select',
      '#title' => t('Sorting field'),
      '#default_value' => isset($old_data['sort']) ? $old_data['sort'] : 0,
      '#options' => simplenews_autoresponder_get_sort(),
      '#description' => t("Which field to use for ordering newsletter issues."),
      '#required' => TRUE,
  );

  $form['item_id'] = array(
      '#type' => 'hidden',
      '#value' => $id,
  );

  $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
  );

  $form['cancel'] = array(
      '#type' => 'button',
      '#value' => t('Cancel'),
      '#name' => 'cancel',
  );

  return $form;
}

function simplenews_autoresponder_admin_edit_submit(&$form, &$form_state) {
  // Prepare data for DB.
  $insert = array(
      'name' => $form_state['values']['name'],
      'tid' => $form_state['values']['tid'],
      'send_interval' => $form_state['values']['interval'],
      'status' => $form_state['values']['enabled'],
      'sort' => $form_state['values']['sort'],
  );

  // Write to DB.
  $db_response = '';
  if ($form_state['values']['item_id'] == -1) {
    $db_response = drupal_write_record('simplenews_autoresponders', $insert);
  }
  else {
    $insert['id'] = $form_state['values']['item_id'];
    $db_response = drupal_write_record('simplenews_autoresponders', $insert, array('id'));
  }

  // Verbose message.
  if ($db_response) {
    drupal_set_message(t("Autoresponder <em>@name</em> was successfully saved.", array('@name' => $form_state['values']['name'])));
  }
  else {
    drupal_set_message(t("An error occured while trying to save <em>@name</em>.", array('@name' => $form_state['values']['name'])), 'error');
  }

  // Redirect back to basic configuration.
  $form_state['redirect'] = 'admin/settings/simplenews_autoresponders';
}

/**
 * Delete callback.
 */
function simplenews_autoresponder_admin_delete(&$form_state, $id) {
  $result = db_query("SELECT name FROM {simplenews_autoresponder} WHERE id = %d", $id);
  $result = db_fetch_array($result);

  $form['id'] = array(
      '#type' => 'value',
      '#value' => $id,
  );

  return confirm_form($form, t('Are you sure you want to delete %title?', array('%title' => $result['name'])), isset($_GET['destination']) ? $_GET['destination'] : 'admin/settings/simplenews_autoresponder', t('This action cannot be undone.'), t('Delete'), t('Cancel')
  );
}

function simplenews_autoresponder_admin_delete_submit(&$form, &$form_state) {
  if ($form_state['values']['confirm']) {
    db_query("UPDATE {simplenews_autoresponder} SET status=2 WHERE id=%d", $form_state['values']['id']);
  }

  $form_state['redirect'] = 'admin/settings/simplenews_autoresponder';
}
