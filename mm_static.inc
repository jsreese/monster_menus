<?php
// $Id: mm_static.inc 4965 2011-01-26 19:22:36Z dan $

/**
 * @file
 * A collection of large, static strings--such as Javascript and HTML code--in MM
 */

/**
 * Return (or add to the current page) some Javascript/HTML/text used in MM
 *
 * @param $which
 *   Key of the code to return/add
 * @param $add_js
 *   If TRUE, add the code to the current page with drupal_add_js()
 * @param $additional
 *   Any additional parameters are passed to the underlying function
 * @return
 *   The Javascript code
 */
function mm_static($which, $add_js = FALSE) {
  static $added = array();

  $args = func_get_args();
  array_splice($args, 0, 2);
  // While this could be done with an array, that would be less efficient.
  // Instead, require that a function of the format "_mm_static__$which" exists
  // and returns some text.
  $name = "_mm_static__$which";
  if (!function_exists($name)) return;
  $code = call_user_func_array($name, $args);

  if (!empty($code) && $add_js && !$added[$which]) {
    drupal_add_js($code, 'inline', 'footer');
    $added[$which] = TRUE;
  }
  return $code;
}

/* Referenced by mm_ui_content_edit */
function _mm_static__edit_cat() {
  return <<<EOJ
Drupal.behaviors.MMEditCat = function(context) {
  \$('.settings-archive:not(.settings-archive-processed)', context)
    .addClass('settings-archive-processed')
    .each(function() {
      var outerDiv = this;
      if (\$('.form-item .form-checkbox', \$(this).parent())
        .click(function() {
          \$(outerDiv).slideToggle('fast');
        }).is(':checked')) {
          \$(this).show();  // init
      }
    });
  \$('.settings-node-types:checkbox:not(.settings-node-types-processed)', context)
    .addClass('settings-node-types-processed')
    .each(function() {
      var outerDiv = $(this).parents('fieldset')[0];
      if (\$(this)
        .click(function() {
          \$('select', outerDiv).attr('disabled', this.checked);
        }).is(':checked')) {
          \$('select', outerDiv).attr('disabled', true);  // init
      }
    });
};
EOJ
  . _mm_static__settings_perms();
}

/* Referenced by mm_ui_content_edit, _mm_ui_node_form_perms */
function _mm_static__settings_perms() {
  global $_mmlist_instance;

  mm_dep_add_libs('xregexp');   // Include fixed version of split()
  $elts = monster_menus_elements();
  $root = $elts['mm_grouplist']['#mm_list_browser'];
  $popup_URL = mm_content_groups_mmtid();
  $popup_base = base_path() . "$root/$popup_URL-" . MM_BROWSER_MODE_GROUP . "-$_mmlist_instance-" . $elts['mm_grouplist']['#mm_list_enabled'] . '-' . $elts['mm_grouplist']['#mm_list_selectable'] . '/';
  $add_groups_label = t('Add a group to the permissions');
  $_mmlist_instance++;

  return <<<EOJ
Drupal.behaviors.MMSettingsPerms = function(context) {
  \$('#mm-permissions:not(.mm-permissions-processed)', context)
    .addClass('mm-permissions-processed')
    .each(function() {
      \$(':checkbox:not(#edit-node-everyone)', this)
        .change(function(event, recurs) {
          var id = this.name.match(/^(\w+)-(\w)-(\w+)\$/);
          var row = \$(this).closest('tr');
          var tbody = \$(this).closest('tbody');
          var list;
          if (id[3] == 'everyone') {
            // Everything else in column
            list = \$(':checkbox:not(.mm-permissions-disabled)[name*=-' + id[2] + ']:not([name=' + this.name + '])', tbody);
            if (this.checked) list.attr('checked', true);
            list
              .toggleClass('mm-permissions-col-disabled', this.checked)
              .attr('disabled', this.checked);
            if (!recurs) {
              \$(':checkbox:checked:not([name=' + this.name + '])', tbody)
                .trigger('change', true);
            }
          }

          if (id[2] != 'r') {
            if (id[2] == 'w') {
              // The rest of the row
              list = \$(':checkbox:not([name=' + this.name + '],.mm-permissions-disabled,.mm-permissions-col-disabled)', row);
            }
            else {
              // r
              list = \$(':checkbox[name*=-r-]:not(.mm-permissions-disabled,.mm-permissions-col-disabled)', row);
            }

            list.attr('disabled', this.checked);
            if (this.checked) list.attr('checked', true);
            else if (!recurs) {
              \$(':checkbox:checked[name^=' + id[1] + '-]:not([name=' + this.name + '],[name*=-r-])', row)
                .trigger('change', true);
            }
          }
        });

      // node perms "everyone"
      \$('#edit-node-everyone', this)
        .change(function() {
          if (\$(this).is(':checked')) {
            \$(this)
              .closest('tbody')
              .children('tr.mm-permissions-data-row')
              .fadeOut(500, function() {
                \$(this).remove();
              });
          }
        });

      // initial setup
      \$('#mm-permissions-group-new-wrapper,#mm-permissions-user-new-wrapper', this)
        .closest('tr')
        .hide()
        .next()
        .andSelf()
        .addClass('mm-permissions-new-row');
      \$('#mm-permissions-group-new-wrapper', this)
        .closest('tr')
        .addClass('mm-permissions-group-new');
      \$('#mm-permissions-user-new-wrapper', this)
        .closest('tr')
        .addClass('mm-permissions-user-new');

      // on submit, process collected data into single fields
      var context = this.parent;
      \$(this)
        .closest('form')
        .find('#edit-submit')
        .click(function() {
          var data = {user: '', group: ''};
          \$(this)
            .closest('form')
            .find('.mm-permissions-data-row :checkbox:checked:visible:enabled,.mm-permissions-data-row :input[type=hidden]')
            .each(function() {
              var id = this.name.match(/^(\w+)-(\w)-(\w+)\$/);
              if (id && id[3] != 'everyone') data[id[1]] += id[2] + id[3];
            });
          $('#edit-all-values-user', context).val(data.user);
          $('#edit-all-values-group', context).val(data.group);
        });
      // fire all the checkbox change events, to handle disabling
      \$(':checkbox:checked', this).each(function() {
        \$(this).trigger('change');
      });
    });
};

Drupal.MMSettingsPermsDelete = function(obj) {
  \$(obj).closest('tr').fadeOut(500, function() {
    \$(this).remove();
  });
  return false;
};

Drupal.MMSettingsPermsAddUsers = function(mmListObj, element) {
  if (mmListObj.length) {
    tb_remove();
    var i = 1, matches = mmListObj.val().split(/(.*?)\{(.*?)\}/);
    var hidden_row = \$('.mm-permissions-user-new:hidden');
    var obj = \$('#mm-permissions [id^=edit-user-r-]:checkbox:visible:not(#edit-user-r-owner):last').closest('tr');
    // If no previous row or limit_write is set, use default row as source of copy
    if (!obj.length || \$('[name=limit_write_not_admin]').length) obj = hidden_row;
    var new_row = [], dups = [];
    for (; i < matches.length; i += 3) {
      var dup = \$('.mm-permissions-data-row :checkbox[name=user-r-' + matches[i] + '],.mm-permissions-data-row :input[type=hidden][name=user-w-' + matches[i] + ']');
      if (dup.length) {
        \$.merge(dups, dup);
      }
      else {
        \$.merge(new_row,
          \$(obj)
            .clone(true)
            .addClass('mm-permissions-new-row')
            .addClass('mm-permissions-data-row')
            .find('td:eq(0) div') // set user's name
              .html(matches[i + 1])
              .end()
            .find(':checkbox,:input[type=hidden]')  // rename checkboxes using uid
              .each(function() {
                $(this).attr('name', this.name.replace(/^(\w+-\w-)(\w+)\$/, '\$1' + matches[i]));
              })
              .end()
            .fadeTo(0, 0)
        );
      }
    }
    mmListObj[0].delAll();
    \$(mmListObj[0].mmList.p.autoCompleteObj).val('');
    Drupal.MMSettingsAnimateDups(dups);
    if (new_row.length) {
      // wait for thickbox to close
      setTimeout(function() {
        \$('#edit-node-everyone').attr('checked', false);
        \$(new_row)
          .insertBefore(hidden_row)
          .show()
          .fadeTo(500, 1);  // fadeIn doesn't work correctly with TRs
        if (obj == hidden_row) {
          // fire all the checkbox change events, to handle disabling
          \$(':checkbox:checked', hidden_row.closest('table')).each(function() {
            \$(this).trigger('change');
          });
        }
      }, 500);
    }
  }
  else {
    tb_remove();
  }
  return false;
};

Drupal.MMSettingsPermsAddGroup = function() {
  url = '$popup_base' + '$popup_URL' + '?TB_iframe=true&height=' + (window.innerHeight - 100) + '&width=' + (window.innerWidth - 100);
  window.mmListInstance = {
    addFromChild: function(chosen, info) {
      var mmtid = chosen.id.substr(5);
      var hidden_row = \$('.mm-permissions-group-new:hidden');
      var obj = \$('#mm-permissions [id^=edit-group-r-]:checkbox:visible:not(#edit-group-r-everyone):last').closest('tr');
      // If no previous row or limit_write is set, use default row as source of copy
      if (!obj.length || \$('[name=limit_write_not_admin]').length) obj = hidden_row;
      var dups = \$('.mm-permissions-data-row :checkbox[name=group-r-' + mmtid + '],.mm-permissions-data-row :input[type=hidden][name=group-w-' + mmtid + ']');
      if (dups.length) {
        tb_remove();
        Drupal.MMSettingsAnimateDups(dups);
      }
      else {
        var new_row = \$(obj)
          .clone(true)
          .addClass('mm-permissions-new-row')
          .addClass('mm-permissions-data-row')
          .find('td:eq(0)')
            .find('fieldset') // make sure fieldset is collapsed
              .addClass('collapsed')
              .find('.fieldset-wrapper')
                .hide()
                .end()
              .end()
            .find('legend a') // copy user list
              .text(\$('a:first', chosen).html().replace(/<(\\w+).*?>.*?<\\/\\1>/g, '').replace(/^\s*(.*?)\s*$/, '$1'))   // IE doesn't support trim()
              .end()
            .find('legend + div div') // copy group name
              .html(info)
              .end()
            .end()
          .find(':checkbox,:input[type=hidden]')  // rename checkboxes using mmtid
            .each(function() {
              $(this).attr('name', this.name.replace(/^(\w+-\w-)(\w+)\$/, '\$1' + mmtid));
            })
            .end()
          .fadeTo(0, 0);
        tb_remove();
        // wait for thickbox to close
        setTimeout(function() {
          \$('#edit-node-everyone').attr('checked', false);
          \$('a.thickbox', new_row)
            .click(function() {
              tb_show(this.title || this.name || null, this.href || this.alt, this.rel || false);
              this.blur();
              return false;
            });
          new_row
            .insertBefore(hidden_row)
            .show()
            .fadeTo(500, 1);
        }, 500);
      }
    }
  };
  tb_show('$add_groups_label', url, false);
  return false;
};

Drupal.MMSettingsAnimateDups = function(dups) {
  dups = \$(dups).closest('tr');
  if (dups.length) {
    // wait for thickbox to close
    setTimeout(function() {
      var old = dups.css('background-color');
      dups.css('background-color', '#ff5').fadeTo(200, 0.25).fadeTo(200, 1).fadeTo(200, 0.25).fadeTo(200, 1).fadeTo(200, 0.25).fadeTo(200, 1, function() {\$(this).css('background-color', old)});
    }, 500);
  }
}

Drupal.MMSettingsPermsOwner = function(mmListObj, element) {
  if (mmListObj.length) {
    var matches = mmListObj.val().split(/(.*?)\{(.*?)\}/);
    if (matches.length > 1) {
      \$('#mm-permissions .settings-perms-owner-name').text(matches[2]);
      \$('[name=owner]').val(matches[1]);
    }
  }
  tb_remove();
  return false;
};
EOJ
  ;
}

/* Referenced by mm_node_redir_form */
function _mm_static__node_redir() {
  return <<<EOJ
Drupal.behaviors.MMNodeRedir = function(context) {
  \$('.node-redir-url:not(.node-redir-url-processed)', context)
    .addClass('node-redir-url-processed')
    .change(function() {
      if (this.value != '' && this.form['redir_category'].value != '')
        this.form['redir_category'].delAll();
    });
};
EOJ
  ;
}

/* Referenced by mm_node_gallery_form */
function _mm_static__node_gallery() {
  return <<<EOJ
Drupal.behaviors.MMNodeGallery = function(context) {
  \$('.node-gallery-appearance:not(.node-gallery-appearance-processed)', context)
    .addClass('node-gallery-appearance-processed')
    .each(function() {
      var outerDiv = this;
      \$('.node-gallery-mode', this).click(function() {
        if (this.checked) {
          \$('#grid_opts,#flow_opts,#link_opts,#single_opts,#1rand_opts,#list_opts', outerDiv).hide();
          \$('#' + this.value + '_opts', outerDiv).show();

          if (this.value == 'list') \$('#edit-show-profile-wrapper', outerDiv).hide();
          else \$('#edit-show-profile-wrapper', outerDiv).show();
        }
      });
      \$('.node-gallery-mode:input:checked', this).click();
    });
};
EOJ
  ;
}

/* Referenced by mm_node_alert_add_to_footer */
function _mm_static__node_alert() {
  return <<<EOJ
Drupal.behaviors.MMNodeAlert = function(context) {
  \$('.mm-alert:not(.mm-alert-processed):first', context)
    .addClass('mm-alert-processed')
    .each(function() {
      \$(':button').click(function() {
        if (\$('.mm-alert:not(.mm-alert-processed)').length) {
          \$('#TB_window').fadeOut('fast', function() {
            \$("#TB_ajaxContent").empty();
            Drupal.behaviors.MMNodeAlert(document); // show next alert while reappearing
          });
          \$('#TB_window').fadeIn('fast');
        }
        else tb_remove();
        return false;
      });
      tb_show(null, "#TB_inline?height=250&width=300&inlineId=" + this.id + "&modal=true", false);
    });
};
EOJ
  ;
}

/* Referenced by mm_ui_content_copymove */
function _mm_static__copymove($mustcheck) {
  return <<<EOJ
Drupal.MMHideNameDiv = function() {
  if (\$('input[name=mode]')[0].checked)
    \$('#namediv').toggle(\$('#edit-copy-page')[0].checked);
  else {
    var x = \$('input[name=move_mode]');
    \$('#namediv').toggle(x.length == 2 && x[0].checked);
  }
  return true;
};

Drupal.MMHideCopyDesc = function() {
  !\$('#edit-copy-page')[0].checked && \$('#edit-copy-nodes')[0].checked ? \$('#copydiv .description').show() : \$('#copydiv .description').hide();
};

Drupal.behaviors.MMCopyMove = function(context) {
  \$('form.mm-ui-copymove:not(.mm-ui-copymove-processed)', context)
    .addClass('mm-ui-copymove-processed')
    .each(function() {
      \$('input[name=mode]', this)
        .click(function() {
          \$('#copydiv').toggle(this.value != 'move');
          \$('#movediv').toggle(this.value == 'move');
          return Drupal.MMHideNameDiv();
        });
      \$('input[name=copy_page]')
        .click(function() {
          if (!this.checked && (!\$('#edit-copy-nodes').length || !\$('#edit-copy-nodes')[0].checked)) {
            {$mustcheck}
            return false;
          }
          var x = \$('#edit-copy-subpage').attr('disabled', !this.checked).parent();
          this.checked ? x.removeClass('disabled') : x.addClass('disabled');
          Drupal.MMHideCopyDesc();
          return Drupal.MMHideNameDiv();
        });
      \$('input[name=copy_nodes]')
        .click(function() {
          if (!this.checked && !\$('#edit-copy-page').attr('checked')) {
            {$mustcheck}
            return false;
          }
        });
      \$('input[name=move_mode]')
        .click(Drupal.MMHideNameDiv);
      Drupal.MMHideCopyDesc();
      \$('input[name=mode]:checked')
        .click();
    });
};
EOJ
  ;
}

/* Referenced by theme_mm_archive */
function _mm_static__select_menu() {
  return <<<EOJ
Drupal.selectMenuAddSelect = function(ul, div, preselect) {
  var select = \$('<select />')
    .change(function(event, preselect) {
      \$(this).nextAll().remove();
      var href = this.value;  // \$(this).val() doesn't work in IE
      if (href && href[0] != '~')
        if (window.parent) window.parent.location = href;
        else document.location = href;
      else {
        \$('.select-menu-no-val', this).remove();
        \$(\$(':selected', this)[0].li).next('ul')
          .each(
            function() {
              Drupal.selectMenuAddSelect(this, div, preselect);
            }
          );
      }
    })
    .appendTo(div);

  if (!\$('.select-menu-no-val').length, select)
    \$('<option class="select-menu-no-val" value="">' + Drupal.t('(choose)') + '<' + '/option>')
      .appendTo(select);

  var kids = 0, kid_val;
  \$(ul)
    .children('li')
      .each(function() {
        kids++;
        var a = \$('a', this);
        kid_val = a.length ? a[0].href : '~' + \$(this).text();
        \$('<option>' + \$(this).text() + '<' + '/option>')
          .appendTo(select)
          .val(kid_val)
          [0].li = this;
      });

  if (kids == 1 && preselect && !kid_val) {
    \$('.select-menu-no-val', select).remove();
    if (select.val() == '') select.trigger('change', [true]);
  }
}

Drupal.behaviors.selectMenuInit = function(context) {
  \$('ul.select-menu:not(.selectMenuInit-processed)', context)
    .addClass('selectMenuInit-processed')
    .hide()
    .each(function() {
      var div = \$('<div class="select-menu"/>').insertAfter(this);
      // pre-select active path
      var list = \$('a.active', this)
        .parents('ul');
      Drupal.selectMenuAddSelect(this, div, list.length == 0);
      // traverse the list backwards
      if (list.length) {
        var select = \$('select', div);
        for (var i = list.length - 1; --i >= 0 && select.length;) {
          select.val('~' + \$(list[i]).prev().text());
          select.trigger('change');
          select = select.next();
        }

        if (select.length) {
          select.val(\$('a.active', this)[0].href);
          \$('.select-menu-no-val', select).remove();
        }
      }
    });
}
EOJ
  ;
}

/* Referenced by theme_mm_tabbed_ui */
function _mm_static__tabbed_ui_js($selected) {
  return <<<EOJ
Drupal.behaviors.MMTabbedUI = function(context) {
  \$('.mm-tabbed-ui:not(.mm-tabbed-ui-processed)', context)
    .addClass('mm-tabbed-ui-processed')
    .each(function() {
      \$('.mm-tabs', this)
        .tabs({selected: $selected});
      var parent = this;
      \$('h5', this)
        .click(function() {
          \$('.mm-tabs', parent).toggle();
        });
      \$('.mm-tabs', this).show();
    });
};
EOJ;
}

/* Referenced by mm_ui_solver */
function _mm_static__solver_mm_list_callback($mmtid) {
  return <<<EOJ
Drupal.mmSolverCallback = function(mmList) {
  var mmtid;
  var val = \$(mmList.hiddenElt).val();
  \$('#mm-solver-table')
    .slideUp('fast', function() {
      if (val && (mmtid = val.match(/^(\d+)\{/)) && mmtid[0] != '') {
        \$.get(Drupal.settings.basePath + 'mm/$mmtid/settings/solver/' + mmtid[1], function(data) {
            \$('#mm-solver-table')
              .html(data)
              .slideDown('fast');
        });
      }
    });
};
EOJ;
}

/* Referenced by mm_browser_bookmark_add */
function _mm_static__mm_browser_bookmark_add($name, $mmtid) {
  global $base_url;

  $base = base_path();
  $mm_path = $base . drupal_get_path('module', 'monster_menus');
  $title = t('Title:');
  $add = t('Add');
  $cancel = t('Cancel');
  $name = check_plain($name);
  $title_msg = t('Please enter a title for this bookmark.');
  return <<<EOH
<html><head>
  <link rel="stylesheet" href="$mm_path/mm_browser_tb.css" />
  <script type="text/javascript" src="$base_url/misc/jquery.js"></script>
  <script type="text/javascript" src="$base_url/sites/all/modules/thickbox/thickbox.js"></script>
</head>
<body id="tb-body">
  <div id="message"><p>$title_msg</p></div>
  <div id="add-bookmark-div">
    <form action="#" onsubmit="return parent.Drupal.mm_browser_add_bookmark_submit(document);">
      <table border="0" cellpadding="3" cellspacing="3" style="margin:0 auto;" >
        <tr>
          <td><label>$title</label></td>
          <td>
            <input name="linktitle" id="linktitle" type="text" size="25" maxlength="35" value="$name" tabindex="1">
            <input name="linkmmtid" type="hidden" value="$mmtid">
          </td>
        </tr>
        <tr align="right">
          <td colspan="2">
            <input type="submit" id="Add" value="$add" onclick="return parent.Drupal.mm_browser_add_bookmark_submit(document);" tabindex="2">&nbsp;
            <input type="submit" id="Cancel" value="$cancel" onclick="parent.tb_remove(); return false;" tabindex="3">
          </td>
        </tr>
      </table>
    </form>
  </div>
</body></html>
EOH;
}

/* Referenced by mm_browser_bookmark_manage */
function _mm_static__mm_browser_bookmark_manage() {
  global $base_url;

  return <<<EOH
\$(function() {
  \$("#sortable").sortable({
    axis:                 'y',
    containment:          'document',
    placeholder:          'ui-state-highlight',
    forcePlaceholderSize: true,
    opacity:              0.5,
    stop:                 function(event, ui) {
      var serialStr = "";
      \$("ul#sortable li").each(function() {
        serialStr += \$(this).attr("name") + "|";
      });
      \$.post(window.parent.Drupal.mm_browser_append_params("$base_url/mm-bookmarks/sort"),
        {neworder: serialStr.substring(0, serialStr.length - 1)},
        function(data) {
          window.parent.Drupal.mm_browser_get_bookmarks();
        },
        "json");
    }
  });
  \$("#sortable").disableSelection();
});
EOH;
}

function _mm_static__back_in_history() {
  return <<<EOH
Drupal.mm_back_in_history = function(save) {
  if (save) {
    document.cookie = 'mm_last_page=' + encodeURI(document.location) + ';path=/';
  }
  else {
    var matches = document.cookie.match(/\bmm_last_page=(.*?)(;|$)/);
    if (matches && matches.length && matches[1].length) {
      var date = new Date(0);
      document.cookie = "mm_last_page=;expires=" + date.toUTCString() + ";path=/";
      document.cookie = 'goto_last=1;path=/';
      document.location = matches[1];
    }
    else {
      document.cookie = 'goto_last=1;path=/';
      window.history.back(-1);
    }
    return false;
  }
};
EOH;
}

/* Referenced by _mm_ui_content_edit */
function _mm_static__ui_content_edit_js($mmtid, $type, $element_name) {
  $clean_element = str_replace('-', '_', $element_name);
  $id_name = str_replace('_', '-', $element_name);
  $columns = _mm_ui_userlist_get_headers();
  $col_def = '';
  for ($i = 0; $i < count($columns) - 1; $i++) {
    $col_def .= 'null, ';
  }
  $col_def .= '{"bSortable": false}';
  $count_text = t('Showing _START_ to _END_ of _TOTAL_ record(s)');
  $filtered_text = t('(filtered from _MAX_ total record(s))');
  switch ($type) {
    case 'group':
      $link_location = "Drupal.settings.basePath + '.mm-users-json/" . $mmtid . "/listusers/" . $element_name . "'";
      $delete_location = "Drupal.settings.basePath + '.mm-users-json/" . $mmtid . "/deleteusers/' + uid";
      $add_location = "Drupal.settings.basePath + '.mm-users-json/" . $mmtid . "/addusers/' + uids";
      break;
    default:
      $link_location = '';
      $delete_location = '';
      $add_location = '';
      break;
  }
  return <<<EOJ
Drupal.behaviors.MMUserDataTable$clean_element = function(context) {
  \$('#mm-user-datatable-$id_name:not(.mm-user-processed)', context).addClass('mm-user-processed').each(function() {
    Drupal.mmgroupTable$clean_element = \$('#mm-user-datatable-$id_name').dataTable({
      aoColumns:      [$col_def],
      bProcessing:    true,
      bServerSide:    true,
      iDisplayLength: 20,
      bLengthChange:  false,
      sAjaxSource:    $link_location,
      oLanguage:      {sInfo: "$count_text", sInfoFiltered: "$filtered_text"}
    });
  });
};
Drupal.mmGroupRemoveUser = function(uid, element) {
  \$.ajax({
    url:     $delete_location,
    async:   false,
    success: function(msg) {
      var table = eval('Drupal.mmgroupTable' + element);
      table.fnDraw();
    }
  });
};
Drupal.mmGroupAddUser = function(mmListObj, element) {
  var uids;
  if (mmListObj.length && (uids = mmListObj.val())) {
    \$.ajax({
      url:     $add_location,
      async:   false,
      success: function() {
        tb_remove();
        mmListObj[0].delAll();
        \$(mmListObj[0].mmList.p.autoCompleteObj).val('');
        var table = eval('Drupal.mmgroupTable' + element);
        table.fnDraw();
      }
    });
  }
  else {
    tb_remove();
  }
  return false;
};
EOJ;
}