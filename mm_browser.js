// $Id: mm_browser.js 4778 2010-11-12 19:21:29Z dan $
/* Javascript file for the Monster Menus tree browser */

Drupal.behaviors.mm_browser_init = function() {
  Drupal.mm_back_in_history(true);

  // Initialize Vertical Splitter
  $("#mmtree-browse-browser").splitter({
    splitVertical: true,
    resizeToWidth: true,
    cookie:        "vsplitter",
    accessKey:     'I'
  });

  $.jstree._themes = Drupal.settings.basePath + Drupal.settings.dependencyPath + "/jsTree/themes/";

  $('.mm-browser-button').each(function() {
    $(this).menu({
      content:            $(this).siblings('.mm-browser-button-list').html(),
      maxHeight:          180,
      width:              220,
      positionOpts: {
        posX:    'left',
        posY:    'bottom',
        offsetX: 0,
        offsetY: 0
      },
      showSpeed:          0,
      flyOut:             true,
      linkHoverSecondary: false
    });
  });

  $('#TB_iframeContent', parent.document).css({overflow: 'hidden'});

  var startPath = Drupal.settings.startBrowserPath;
  if (document.cookie.indexOf("goto_last=1") >= 0) {
    // User pressed Back button on media properties page
    var date = new Date(0);
    document.cookie = "goto_last=1;expires=" + date.toUTCString() + ";path=/";
    startPath = Drupal.settings.lastBrowserPath;
  }
  Drupal.mm_browser_init_jstree(startPath);
};

Drupal.mm_browser_init_jstree = function(path) {
  var setHeight = function() {
    var ht = $('#mmtree-browse').height() - $('#mmtree-browse-nav').height();
    if (ht > 0) $("#mmtree-browse-browser,#mmtree-browse-tree,#mmtree-browse-iframe").height(ht + 5);
  };

  var initially_open = ('mmbr-' + path.replace(/\//g, '/mmbr-')).split('/');
  var initially_select = initially_open.pop();
  $("#mmtree-browse-tree")
    .jstree({
      core: {
        animation:      200,
        html_titles:    true,
        strings: {
          loading: Drupal.t("Loading...")
        },
        initially_open: initially_open
      },
      json_data: {
        ajax: {
          url:   function(n) {
            var id = n.attr ? n.attr('id').substr(5) : Drupal.settings.browserTop;
            return Drupal.settings.basePath + "mm-browser/" + id + '?' + Drupal.mm_browser_params();
          },
          error: function(x, t, e) {
            alert(x.status == 403 ?
              Drupal.t('You do not have permission to perform this operation.') :
              Drupal.t('An error occurred: !err', { '!err' : x.statusText })
            );
            this.destroy();
            $('#mmtree-browse-tree ul').remove();
          }
        }
      },
      ui: {
        select_limit:     1,
        initially_select: [ initially_select ]
      },
      themes: {
        theme: 'classic'
      },
      plugins: ["themes", "json_data", "ui"]
    })
    .bind("before.jstree", function(e, data) {
      if (data.func == "open_node" || data.func == "select_node") {
        if ($(data.args[0]).is('.disabled')) {
          e.stopImmediatePropagation();
          return false;
        }
        if (data.func === "select_node") {
          var obj = $(data.args[0]);
          if (obj.length) Drupal.mm_browser_refresh_right(obj[0].nodeName == 'A' ? obj.parent()[0] : obj[0]);
        }
      }
    })
    .one("select_node.jstree", function(e, data) {
      // Sometimes setHeight() gets called during startup before the window has
      // been rendered, and it therefore fails. So repeat it once everything has
      // calmed down.
      setHeight();
      // Scroll the parent so the initially_select is in view
      $(e.currentTarget).scrollTop($(data.args[0]).position().top);
    });

  if (window.opener) {
    $(window).resize(setHeight);
  }
  else {
    $("#mmtree-browse-browser,#mmtree-browse-tree,#mmtree-browse-iframe").resize(setHeight);
  }
  setHeight();
}

Drupal.mm_browser_reload_data = function(path) {
  path = path || '1';
  if (!path.match('(^|/)' + Drupal.settings.browserTop + '(/|$)')) {
    Drupal.settings.browserTop = path.split('/')[0];
  }
  $("#mmtree-browse-tree").jstree('.destroy');
  Drupal.mm_browser_init_jstree(path);
};

Drupal.mm_browser_goto_top = function(path) {
  Drupal.settings.browserTop = path.split('/')[0];
  Drupal.mm_browser_reload_data(path);
};

Drupal.mm_browser_params = function() {
  var out = [];
  for (i in Drupal.settings)
    if (i.substring(0, 7) == 'browser' && i.length > 7)
      out.push(i + '=' + encodeURI(Drupal.settings[i]));
  return out.join('&');
}

Drupal.mm_browser_append_params = function(uri) {
  var extra = Drupal.mm_browser_params();
  if (uri.indexOf('?') > 0) return uri + '&' + extra;
  return uri + '?' + extra;
}

Drupal.mm_browser_close_menus = function() {
  for (var i in allUIMenus)
    if (allUIMenus[i].menuOpen) allUIMenus[i].kill();
};

Drupal.mm_browser_last_viewed = function() {
  if (Drupal.settings.lastBrowserPath) {
    Drupal.mm_browser_reload_data(Drupal.settings.lastBrowserPath);
  }
};

Drupal.mm_browser_refresh_right = function(node) {
  ht = $('#mmtree-browse').height() - $('#mmtree-browse-nav').height() + 5;
  $("#mmtree-browse-items").html('<iframe id="mmtree-browse-iframe" width="100%" height="' + ht + '" frameborder="0" src="' + Drupal.settings.basePath + 'mm-browser-getright?id=' + node.id + '&' + Drupal.mm_browser_params() + '"></iframe>');
  $.getJSON(
    Drupal.mm_browser_append_params(Drupal.settings.basePath + "mm-browser-get-lastviewed?id=" + node.id),
    function(data) {
      Drupal.settings.lastBrowserPath = data.path;
    }
  );
};

Drupal.mm_browser_add_bookmark_tb = function(id) {
  tb_show(Drupal.t("Add Bookmark"), Drupal.settings.basePath + "mm-bookmarks/add/" + id + "?" + Drupal.mm_browser_params() + "&TB_iframe=true&modal=true&height=150&width=250");
};

Drupal.mm_browser_add_bookmark_submit = function(context) {
  $("#add-bookmark-div", context).hide();
  var linktitle = $("input[name=linktitle]", context).val();
  var linkmmtid = $("input[name=linkmmtid]", context).val();
  $.post(
    Drupal.mm_browser_append_params(Drupal.settings.basePath + "mm-bookmarks/add"), {
      linktitle: linktitle,
      linkmmtid: linkmmtid},
    function(data) {
      Drupal.mm_browser_get_bookmarks();
      tb_remove();
    }
  );
  return false;
};

Drupal.mm_browser_manage_bookmarks = function() {
  tb_show(Drupal.t("Organize Bookmark"), Drupal.settings.basePath + "mm-bookmarks/manage?" + Drupal.mm_browser_params() + "&TB_iframe=true&height=150&width=250");
};

Drupal.mm_browser_get_bookmarks = function() {
  $.getJSON(
    Drupal.mm_browser_append_params(Drupal.settings.basePath + 'mm-browser-get-bookmarks'),
    function(data) {
      $(".bm-list").empty();
      for (var i in data) {
        $(".bm-list").append('<li class="' + data[i][0] + '"><a href="#" onclick="Drupal.mm_browser_reload_data(\'' + data[i][2] + '\')">' + data[i][1] + '</a></li>');
      }
    }
  );
};

Drupal.mm_browser_delete_bookmark_confirm = function(mmtid, title, context) {
  $("#" + mmtid, context).html('<td class="tb-manage-name"><span class="ui-icon ui-icon-arrowthick-2-n-s"></span>' + Drupal.t('<strong>Are you sure you want to DELETE</strong> %title?', {'%title': title}) + '</td><td><a href="#" onclick="return parent.Drupal.mm_browser_delete_bookmark(' + mmtid + ', document);">' + Drupal.t('Delete') + '</a></td><td><a href="#" onclick="return parent.Drupal.mm_browser_reset_bookmark(' + mmtid + ",'" + title + "', document);\">" + Drupal.t('Cancel') + '</a></td>');
  return false;
};

Drupal.mm_browser_edit_bookmark_edit = function(mmtid, title, context) {
  $("#" + mmtid, context).html('<td class="tb-manage-name"><form action="#" onsubmit="parent.Drupal.mm_browser_edit_bookmark(' + mmtid + ', document); return false;"><span class="ui-icon ui-icon-arrowthick-2-n-s"></span><input type="text" maxlength="35" name="edittitle-' + mmtid + '" value="' + title + '"><input type="hidden" name="editmmtid-' + mmtid + '" value="' + mmtid + '"></form></td><td><a href="#" onclick="return parent.Drupal.mm_browser_edit_bookmark(' + mmtid + ', document);">' + Drupal.t('Save') + '</a></td><td><a href="#" onclick="return parent.Drupal.mm_browser_reset_bookmark(' + mmtid + ', \'' + title + '\', document);">' + Drupal.t('Cancel') + '</a></td>');
  $('input[name=edittitle-' + mmtid + ']', context).focus();
  return false;
};

Drupal.mm_browser_reset_bookmark = function(mmtid, title, context) {
  $("#li_" + mmtid, context).html('<table class="manage-bookmarks-table"><tr id="' + mmtid + '"><td class="tb-manage-name"><span class="ui-icon ui-icon-arrowthick-2-n-s"></span>' + title + '</td><td><a href="#" onclick="return parent.Drupal.mm_browser_delete_bookmark_confirm(' + mmtid + ", '" + title + '\', document);">' + Drupal.t('Delete') + '</a></td><td><a href="#" onclick="return parent.Drupal.mm_browser_edit_bookmark_edit(' + mmtid + ", '" + title + '\', document);">' + Drupal.t('Edit') + '</a></td></tr><table>');
  return false;
};

Drupal.mm_browser_delete_bookmark = function(mmtid, context) {
  $.post(
    Drupal.mm_browser_append_params(Drupal.settings.basePath + "mm-bookmarks/delete"),
    {mmtid: mmtid},
    function(data) {
      $("#li_" + mmtid, context).remove();
      $("." + data.mmtid).remove();
    },
    "json"
  );
  return false;
};

Drupal.mm_browser_edit_bookmark = function(emmtid, context) {
  $.post(
    Drupal.mm_browser_append_params(Drupal.settings.basePath + "mm-bookmarks/edit"), {
      mmtid: $("input[name=editmmtid-" + emmtid + "]", context).val(),
      title: $("input[name=edittitle-" + emmtid + "]", context).val()},
    function(data) {
      Drupal.mm_browser_reset_bookmark(data.mmtid, data.title, context);
      $("." + data.mmtid).children().html(data.title);
    },
    "json"
  );
  return false;
};

Drupal.mm_browser_change_parent_url = function(url) {
  document.location = url;
};

Drupal.mm_browser_gallery_add = function(mmtid, filename, nid) {
  parent && parent.mmListInstance && parent.mmListInstance.addFromChild($("#mmbr-" + mmtid)[0], 0, nid, filename);
  return false;
};

Drupal.mm_browser_nodepicker_add = function(mmtid, nodename, nid) {
  parent && parent.mmListInstance && parent.mmListInstance.addFromChild($("#mmbr-" + mmtid)[0], 0, nid, nodename);
  return false;
};

Drupal.mm_browser_page_add = function(mmtid, info) {
  parent && parent.mmListInstance && parent.mmListInstance.addFromChild($("#mmbr-" + mmtid)[0], info);
};
