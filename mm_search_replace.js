// $Id: mm_search_replace.js 4818 2010-11-22 16:40:23Z dan $
(function($) {
  $.fn.extend({
    mySerialize: function() {
      var s = [];
      for (var i = 0; i < this.length; i++)
        if (this[i].type == 'select-multiple') {
          var l = $('option:selected', this[i]), o = [];
          for (var j = 0; j < l.length; j++)
            if (l[j].selected) o.push(l[j].value);
          s.push(this[i].name.replace(/\[\]$/, '') + '=' + encodeURIComponent(o.join(',')));
        }
        else if (this[i].type == 'checkbox')
          s.push(this[i].name + '=' + encodeURIComponent(this[i].checked + 0));
        else
          s.push(this[i].name + '=' + encodeURIComponent(this[i].value));
      return s.join('&');
    }
  });
})(jQuery);

var MMSR_CONTENTS=       '0';
var MMSR_PAGES=          '1';
var MMSR_CONT_PAGES=     '2';
var MMSR_GROUPS=         '3';
var MMSR_CONTENTS_PANEL=  0;
var MMSR_PAGES_PANEL=     1;
var MMSR_GROUPS_PANEL=    2;

var MMSR_init_done, MMSR_last_recalc;

MMSR_search_type = function( value ) {
  var where = $('#mmsr-where-list');
  // don't use empty() here because it destroys event handlers
  while (where[0].childNodes[0])
    where[0].removeChild(where[0].childNodes[0]);
  switch (value) {
    case MMSR_CONTENTS:
      where.append(MMSR_panels[MMSR_CONTENTS_PANEL]);
      break;
    case MMSR_CONT_PAGES:
      where.append(MMSR_panels[MMSR_CONTENTS_PANEL]);
//    no break
    case MMSR_PAGES:
      where.append(MMSR_panels[MMSR_PAGES_PANEL]);
      break;
    case MMSR_GROUPS:
      where.append(MMSR_panels[MMSR_GROUPS_PANEL]);
  }
}

MMSR_collapse = function( obj ) {
  var tr = $(obj.parentNode.parentNode.nextSibling);
  if ($('div', tr).length) {
    tr.toggle();
    $(obj).toggleClass('collapsed');
  }
  return false;
}

MMSR_where_changed = function() {
  var widgets = $('form div #search-' + this.value + ':not(display=none)')
    .clone(true)
    .show();
  $('.subpanel-select', widgets)
    .change(MMSR_subpanel_changed)
    .each(MMSR_subpanel_changed);
  var search_changed = function() {
    this.selectedIndex == 1 ? $(this).parent().siblings().show() : $(this).parent().siblings().hide();
  }
  $("[name=search-archive-0]", widgets)
    .change(search_changed)
    .each(search_changed);
  $('[name=mmsr-widgets]', this.parentNode.parentNode.nextSibling)
    .empty()
    .append(widgets)
    .parent()
    .show();
  $(':input:visible', widgets)
    .change(MMSR_recalculate);
  var oldMMLists = $('.mm-list-hidden', $('form div #search-' + this.value));
  $('.mm-list-hidden', widgets)
    .each(function(i) {
      this.mmList = oldMMLists[i].mmList;
    });
  for (var i in Drupal.settings.MMSR.fixups)
    $(i + ':not(.subpanel-inited)', widgets)
      .addClass('subpanel-inited')
      .each(Drupal.settings.MMSR.fixups[i]);
  $('[name=mmsr-collapse]', this.parentNode)
    .removeClass('collapsed');
  MMSR_recalculate();
}

MMSR_plus_clicked = function() {
  var tr = this.parentNode.parentNode;
  var trcopy = $(tr)
    .clone(true);
  var from = $('[name=search-logic]', tr);
  $('[name=search-logic]', trcopy)
    .val(from.val())
    .show();
  var pop, trClass;
  switch (tr.className) {
    case 'mmsr-page-row':
      pop = $('[name=search-page-wheres]', trcopy);
      from = $('[name=search-page-wheres]', tr);
      trClass = 'mmsr-page-row-widgets';
      break;
    case 'mmsr-cont-row':
      pop = $('[name=search-node-wheres]', trcopy);
      from = $('[name=search-node-wheres]', tr);
      trClass = 'mmsr-cont-row-widgets';
      break;
    case 'mmsr-group-row':
      pop = $('[name=search-group-wheres]', trcopy);
      from = $('[name=search-group-wheres]', tr);
      trClass = 'mmsr-group-row-widgets';
      break;
  }
  pop
    .val(from.val());
  $(tr)
    .next()
    .after(trcopy)
    .next()
    .after('<tr class="' + trClass + '"><td id="mmsr-widgets" name="mmsr-widgets" colspan="2"></td></tr>');
  $(pop)
    .each(MMSR_where_changed);
  $('[name=mmsr-minus]:hidden', tr.parentNode)
    .show();
}

MMSR_minus_clicked = function() {
  var tr = this.parentNode.parentNode;
  var parent = tr.parentNode;
  $(tr)
    .next()
    .remove();
  $(tr)
    .remove();
  if ($('[name=mmsr-minus]', parent).length == 1)
    $('[name=mmsr-minus]:first:visible', parent)
      .hide();
  $('[name=search-logic]:first:visible', parent)
    .hide();
  MMSR_recalculate();
}

MMSR_subpanel_changed = function() {
  $('[name^=' + this.name + '-]', this.parentNode.parentNode)
    .hide()
    .each(function() {
      $('input', this).addClass('mmsr-ignore');
    }
  );
  $('[name=' + this.name + '-' + this.value + ']', this.parentNode.parentNode)
    .show()
    .each(function() {
      $('input', this)
        .removeClass('mmsr-ignore');
      if (!$(this).hasClass('subpanel-inited')) {
        for (var i in Drupal.settings.MMSR.fixups)
          $(i + ':not(.subpanel-inited)', this)
            .addClass('subpanel-inited')
            .each(Drupal.settings.MMSR.fixups[i]);
        $(this)
          .addClass('subpanel-inited');
      }
    }
  );
  MMSR_recalculate();
}

MMSR_initialize = function() {
  $('#rightcolumn')
    .hide();    // hide the page's right column (admin menu)
  $('#search-logic')
    .change(MMSR_recalculate)
    .parent()
    .hide();
  $('#search-node-type')
    .change(MMSR_recalculate)
    .parent()
    .hide();
  $('#search-node-wheres')
    .change(MMSR_recalculate)
    .parent()
    .hide();
  $('#edit-search-type')
    .change(
      function() {
        MMSR_search_type( this.value );
        MMSR_recalculate();
      }
    );
  $('.subpanel')
    .hide();
  $('#edit-search-archive-3-wrapper label')
    .css('margin-top', '6px');
  $('#search-archive div:gt(0)')
    .css('display', 'inline');

  // run the fixup for "pages starting at"
  var toFind = $('#search-page-catlist,#search-group-catlist'), n = 0;
  for (var i in Drupal.settings.MMSR.fixups) {
    var test = $(i, this);
    var test2 = test[0].parentNode;
    if (test2 == toFind[0] || test2 == toFind[1]) {
      test
        .each(Drupal.settings.MMSR.fixups[i]);
      if (++n == 2) break;
    }
  }

  $('#search-page-wheres')
    .after('<div id="mmsr-where-list"></div>');
  $('#search-group-wheres')
    .after('<div id="mmsr-where-list"></div>');
  MMSR_panels = Array(
    $('<table class="mmsr-cont-table"><tbody>'+
      '<tr class="mmsr-cont-row-widgets"><td colspan="2"><label>' + Drupal.settings.MMSR.t_of_type + ':</label></td></tr>' +
      '<tr class="mmsr-cont-row-widgets"><td colspan="2"></td></tr>' +
      '<tr class="mmsr-cont-row-widgets"><td colspan="2"><label>' + Drupal.settings.MMSR.t_where + ':</label></td></tr>' +
      '<tr name="mmsr-cont-row" class="mmsr-cont-row"><td id="mmsr-where" name="mmsr-where" nowrap="true"></td><td name="mmsr-plus-minus" id="mmsr-plus-minus" width="0"></td></tr>' +
      '<tr class="mmsr-cont-row-widgets"><td id="mmsr-widgets" name="mmsr-widgets" colspan="2"></td></tr>' +
      '</tbody></table>'),
    $('<table class="mmsr-page-table"><tbody>' +
      '<tr class="mmsr-page-row-widgets"><td colspan="2"></td></tr>' +
      '<tr class="mmsr-page-row-widgets"><td colspan="2"><label>' + Drupal.settings.MMSR.t_where + ':</label></td></tr>' +
      '<tr name="mmsr-page-row" class="mmsr-page-row"><td id="mmsr-where" name="mmsr-where" nowrap="true"></td><td name="mmsr-plus-minus" id="mmsr-plus-minus" width="0"></td></tr>' +
      '<tr class="mmsr-page-row-widgets"><td id="mmsr-widgets" name="mmsr-widgets" colspan="2"></td></tr>' +
      '</tbody></table>'),
    $('<table class="mmsr-group-table"><tbody>' +
      '<tr class="mmsr-group-row-widgets"><td colspan="2"></td></tr>' +
      '<tr class="mmsr-group-row-widgets"><td colspan="2"><label>' + Drupal.settings.MMSR.t_where + ':</label></td></tr>' +
      '<tr name="mmsr-group-row" class="mmsr-group-row"><td id="mmsr-where" name="mmsr-where" nowrap="true"></td><td name="mmsr-plus-minus" id="mmsr-plus-minus" width="0"></td></tr>' +
      '<tr class="mmsr-group-row-widgets"><td id="mmsr-widgets" name="mmsr-widgets" colspan="2"></td></tr>' +
      '</tbody></table>')
  );

  // contents panel
  $('tr:first td:first', MMSR_panels[MMSR_CONTENTS_PANEL])
    .append($('#search-node-type'));
  $('[name=mmsr-where]', MMSR_panels[MMSR_CONTENTS_PANEL])
    .append($('<a href="#" name="mmsr-collapse" onclick="return MMSR_collapse(this)"></a>'))
    .append($('#search-logic')
      .clone()
      .change(MMSR_recalculate));
  $('[name=mmsr-where]', MMSR_panels[MMSR_CONTENTS_PANEL])
    .append($('#search-node-wheres')
      .clone()
      .show()
      .change(MMSR_where_changed)
    );
  $('[name=mmsr-plus-minus]', MMSR_panels[MMSR_CONTENTS_PANEL])
    .append('<input type="button" name="mmsr-minus" value="-" title="' + Drupal.settings.MMSR.t_minus + '" style="display: none">')
    .append('<input type="button" name="mmsr-plus" title="' + Drupal.settings.MMSR.t_plus + '" value="+">');
  $('[name=mmsr-plus]', MMSR_panels[MMSR_CONTENTS_PANEL])
    .click(MMSR_plus_clicked);
  $('[name=mmsr-minus]', MMSR_panels[MMSR_CONTENTS_PANEL])
    .click(MMSR_minus_clicked);

  // pages panel
  $('tr:first td:first', MMSR_panels[MMSR_PAGES_PANEL])
    .append($('#search-page-catlist'));
  $('[name=mmsr-where]', MMSR_panels[MMSR_PAGES_PANEL])
    .append($('<a href="#" name="mmsr-collapse" onclick="return MMSR_collapse(this)"></a>'))
    .append($('#search-logic')
      .clone()
      .change(MMSR_recalculate));
  $('[name=mmsr-where]', MMSR_panels[MMSR_PAGES_PANEL])
    .append($('#search-page-wheres')
      .clone()
      .show()
      .change(MMSR_where_changed)
    );
  $('[name=mmsr-plus-minus]', MMSR_panels[MMSR_PAGES_PANEL])
    .append('<input type="button" name="mmsr-minus" value="-" title="' + Drupal.settings.MMSR.t_minus + '" style="display: none">')
    .append('<input type="button" name="mmsr-plus" title="' + Drupal.settings.MMSR.t_plus + '" value="+">');
  $('[name=mmsr-plus]', MMSR_panels[MMSR_PAGES_PANEL])
    .click(MMSR_plus_clicked);
  $('[name=mmsr-minus]', MMSR_panels[MMSR_PAGES_PANEL])
    .click(MMSR_minus_clicked);

  // groups panel
  $('tr:first td:first', MMSR_panels[MMSR_GROUPS_PANEL])
    .append($('#search-group-catlist'));
  $('[name=mmsr-where]', MMSR_panels[MMSR_GROUPS_PANEL])
    .append($('<a href="#" name="mmsr-collapse" onclick="return MMSR_collapse(this)"></a>'))
    .append($('#search-logic'));
  $('[name=mmsr-where]', MMSR_panels[MMSR_GROUPS_PANEL])
    .append($('#search-group-wheres')
      .clone()
      .show()
      .change(MMSR_where_changed)
    );
  $('[name=mmsr-plus-minus]', MMSR_panels[MMSR_GROUPS_PANEL])
    .append('<input type="button" name="mmsr-minus" value="-" title="' + Drupal.settings.MMSR.t_minus + '" style="display: none">')
    .append('<input type="button" name="mmsr-plus" title="' + Drupal.settings.MMSR.t_plus + '" value="+">');
  $('[name=mmsr-plus]', MMSR_panels[MMSR_GROUPS_PANEL])
    .click(MMSR_plus_clicked);
  $('[name=mmsr-minus]', MMSR_panels[MMSR_GROUPS_PANEL])
    .click(MMSR_minus_clicked);

  $('#mm-search-form')
    .append('<div id="mmsr-status"><div id="mmsr-status-text"></div><img src="' + Drupal.settings.MMSR.loading + '" style="display: none"></div>')
    .append('<div id="diagnostic" style="clear: right"></div>');
  $('#mmsr-status #mmsr-status-text')
    .before(
      $('#edit-result')
        .click(function() {
          MMSR_serialize();
          return true;
        }))
    .before(
      $('#edit-reset')
        .click(function() {
          MMSR_init_done = false;
          $('input[type=button][name=mmsr-minus]:gt(0)', MMSR_panels[MMSR_CONTENTS_PANEL])
            .click();
          $('input[type=button][name=mmsr-minus]:gt(0)', MMSR_panels[MMSR_PAGES_PANEL])
            .click();
          $('input[type=button][name=mmsr-minus]:gt(0)', MMSR_panels[MMSR_GROUPS_PANEL])
            .click();
          MMSR_import(Drupal.settings.MMSR.reset, document);
          MMSR_init_done = true;
          MMSR_recalculate();
          return false;
        }));

  MMSR_import(Drupal.settings.MMSR.startup, document);
  $("[name=search-page-cat]", MMSR_panels[MMSR_PAGES_PANEL])
    .val(Drupal.settings.MMSR.startup['search-page-cat'])
    .trigger('change');
  $("[name=search-group-cat]", MMSR_panels[MMSR_GROUPS_PANEL])
    .val(Drupal.settings.MMSR.startup['search-group-cat'])
    .trigger('change');
  MMSR_init_done = true;
  MMSR_recalculate();
}

MMSR_import = function(obj, where) {
  if (typeof(obj) == 'object')
    if (typeof(obj.length) == 'number') { // array
      for (var i in obj) {
        var row = where;
        if (i > 0) {
          $('[name=mmsr-plus]:last', row)
            .trigger('click');
          row = row.nextSibling.nextSibling;
        }
        $(row)
          .each(
            function() {
              MMSR_import_inner(obj[i], this)
            });
      }
    }
    else {
      for (var i in obj) {
        var row = where;
        if (row.tagName == 'TR' && i != 'search-logic' && i != 'search-node-wheres' && i != 'search-page-wheres' && i != 'search-group-wheres')
            row = row.nextSibling;
        $('[name="' + i + '"]:first', row)
          .each(
            function() {
              MMSR_import_inner(obj[i], this);
            });
      }
    }
}

MMSR_import_inner = function(obji, where) {
  if (typeof(obji) == 'object')
    MMSR_import(obji, where);
  else {
    where.value = obji;
    if (where.tagName == 'SELECT' || where.tagName == 'INPUT')
      $(where).trigger('change');
  }
}

MMSR_serialize = function() {
  if (MMSR_init_done) {
    var data = $($.merge($.merge(jQuery.makeArray(
      $("#edit-search-type,#mmsr-where-list table :input:not(:button):not(:submit):not([style='display: none']):not(.mmsr-ignore):not(.autocomplete):not([name=search-page-cat]):not([name=search-group-cat])")),
      $("[name=search-page-cat]", MMSR_panels[MMSR_PAGES_PANEL])),
      $("[name=search-group-cat]", MMSR_panels[MMSR_GROUPS_PANEL])))
      .mySerialize();
    if (data != MMSR_last_recalc) {
      $('#edit-data')
        .val(data);
      MMSR_last_recalc = data;
      return data;
    }
  }
  return false;
}

MMSR_recalculate = function() {
  var data = MMSR_serialize();
  if (data) {
    $('#mmsr-status-text')
      .hide()
      .next()
      .show();
    $.ajax({
      type:     'POST',
      dataType: 'json',
      data:     { data: data },
      url:      Drupal.settings.MMSR.get_path,
      global:   false,
      success:  function(obj) {
                  $('#mmsr-status-text')
                    .html(obj.result)
                    .show()
                    .next()
                    .hide();
                  $('#diagnostic')
                    .html('<font color="gray">' + (obj.query || '') + '</font>');
                },
      error:    function(a,b,c) {
                  $('#mmsr-status-text')
                    .html('error')
                    .show()
                    .next()
                    .hide();
                  $('#diagnostic')
                    .html('<font color="gray">' + (obj.query || '') + '</font>');
                }
    });
  }
}

if (Drupal.jsEnabled) {
  $(document)
    .ready(MMSR_initialize);
  // deactivate built-in autoattach function
  for (var i in jQuery.readyList)
    if (jQuery.readyList[i] == Drupal.autocompleteAutoAttach) jQuery.readyList[i] = function() {};
}

/*********** Fixup code for various custom element types ***********/

var MMSR_acdb = [];
MMSR_fixup_autocomplete = function() {
  var uri = this.value;
  if (!MMSR_acdb[uri])
    MMSR_acdb[uri] = new Drupal.ACDB(uri);
  var input =
    $('input.form-autocomplete', this.parentNode)
      .attr('autocomplete', 'OFF')[0];
  $(input.form)
    .submit(Drupal.autocompleteSubmit);
  new Drupal.jsAC(input, MMSR_acdb[uri]);
}
