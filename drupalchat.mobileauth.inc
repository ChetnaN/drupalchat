<?php



function drupalchat_mobile_auth()
{

    if (variable_get('drupalchat_enable_mobile_sdk_integration', '2') == '1') {
            $form_state = array(
                'values' => array
                (
                    'name' => $_POST['username'],
                    'pass' => $_POST['password'],
                    'submit' => 'Log in',
                    'form_build_id' => 'form-' . drupal_random_key(),
                    'form_id' => 'user_login_block',
                    'op' => 'Log in',
                ),
				);
                
            user_login_authenticate_validate(null, $form_state);
            if (empty($form_state['uid'])) {
                // Always register an IP-based failed login event.
                flood_register_event('failed_login_attempt_ip', variable_get('user_failed_login_ip_window', 3600));
                // Register a per-user failed login event.
                if (isset($form_state['flood_control_user_identifier'])) {
                    flood_register_event('failed_login_attempt_user', variable_get('user_failed_login_user_window', 21600), $form_state['flood_control_user_identifier']);
                }

                if (isset($form_state['flood_control_triggered'])) {
                    if ($form_state['flood_control_triggered'] == 'user') {
                        print_r('Temporarily blocked.');
                    } else {
                        // We did not find a uid, so the limit is IP-based.
                        print_r('IP blocked');
                    }
                } else {
                    print_r('Wrong password');
                }
            } elseif (isset($form_state['flood_control_user_identifier'])) {
                // Clear past failures for this user so as not to block a user who might
                // log in and out more than once in an hour.
                flood_clear_event('failed_login_attempt_user', $form_state['flood_control_user_identifier']);
                $user = user_load($form_state['uid']);
                if (user_access('access drupalchat', $user)) {
                    global $base_url;
                    if (user_access('administer drupalchat', $user)) {
                        $role = "admin";
                    } else {
                        $role = $user->roles;
                    }

                    $data = array(
                        'uname' => $_POST['username'],
                        'uid' => $form_state['uid'],
                        'api_key' => variable_get('drupalchat_external_api_key', NULL),
                        'image_path' => $base_url . '/' . drupal_get_path('module', 'drupalchat') . '/themes/' . variable_get('drupalchat_theme', 'light') . '/images',
                        'isLog' => TRUE,
                        'whichTheme' => 'blue',
                        'enableStatus' => TRUE,
                        'role' => $role,
                        'validState' => array('available', 'offline', 'busy', 'idle'),
                        'rel' => variable_get('drupalchat_rel', DRUPALCHAT_REL_AUTH),
                    );
                    //Added allRoles if role is admin.
                    if ($role == 'admin') {
                        $data['allRoles'] = user_roles();
                    }
                    if (variable_get('drupalchat_user_picture', 1) == 1) {
                        $data['up'] = drupalchat_return_pic_url($form_state['uid']);
                    }

                    $data['upl'] = drupalchat_return_profile_url();

                    $data = json_encode($data);
                    $options = array(
                        'method' => 'POST',
                        'data' => $data,
                        'timeout' => 15,
                        'headers' => array('Content-Type' => 'application/json'),
                    );

                    $result = drupal_http_request(DRUPALCHAT_EXTERNAL_A_HOST . ':' . DRUPALCHAT_EXTERNAL_A_PORT . '/p/', $options);
                    if ($result->code == 200) {
                        drupal_json_output(drupal_json_decode($result->data));
                    } else {
                        print_r('sever error');
                    }
                } else {
                    drupal_access_denied();
                }


            }
        } else {
            print_r("Please Enable Mobile SDK Integration");
        }



}
