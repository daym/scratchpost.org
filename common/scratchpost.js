
/* this is a weird hack but I can't think of anything better to get 
   the darn menu to work without resorting to fixed widths */

function fix_menu_width()
{
  if (document.getElementById != null && false) {
    var main_border = document.getElementById("main_border");
    var main_navigation = document.getElementById("main_navigation");
    if (main_border != null && main_navigation != null/* && main_border.position == "absolute"*/) {
      main_navigation.style.minWidth = "15em";
      var main_navigation_bottom = main_navigation.offsetTop + main_navigation.offsetHeight;

      //main_border.style.position = "relative";
      //main_border.style.left = 0;

      if (main_border.offsetTop < main_navigation_bottom) { 
        /* content to the right of the navigation, not below */
        //main_border.style.position = "absolute";
        main_border.style.left = (main_navigation.offsetLeft + main_navigation.offsetWidth) + "px";
      } else {
      }
    }
  }
}

old_onload = window.onload;
old_onresize = window.onresize;

function fix_menu_width_cb()
{
  fix_menu_width(); 
  if (old_onload != null) {
    return old_onload();
  }
}

function fix_menu_width_resize_cb()
{
  fix_menu_width();
  if (old_onresize != null) {
    return old_onresize();
  }
}

window.onload = fix_menu_width_cb;
window.onresize = fix_menu_width_resize_cb;
//window.setTimeout("fix_menu_width_cb()", 100);

// expander
// --------------------- Variables --------------------------
// Default values for image names of the images/text to expand and collapse the hidden text.
// ----------------------------------------------------------
//var expandImage = "expand.gif"; 
//var collapseImage = "collapse.gif"; 
var defaultExpandText = "Mehr Details zeigen";
var defaultCollapseText = "Weniger Details zeigen";
// ---------------- setDefaultExpanderText(...) ------------------
// Call this method to change the strings used for the "expand" and "collapse" links
// expandText = "Expand" or "Show More" or "Reveal Answer" etc.
// collapseText = "Hide" or "Show Less" or "Hide Answer" etc.
// ----------------------------------------------------------
function setDefaultExpanderText(expandText, collapseText) {
    defaultExpandText = expandText;
    defaultCollapseText = collapseText;
}

function toggleExpanderById(hiddenDivId, expander, expandText, collapseText) {
    if(document.getElementById)
        toggleExpander(document.getElementById(hiddenDivId), expander, expandText, collapseText);
}
function animateExpander(div, bOpening) {
    if(bOpening) {
        var n = parseFloat(div.style.height) + 3;
        if(n > div.originalHeight)
            n = div.originalHeight;
        div.style.height = n + " px";
        if(n >= div.originalHeight) { // done opening
            if(div.closeTimer) {
                clearInterval(div.closeTimer);
                div.closeTimer = 0;
            }
        }
    } else {
        var n = parseFloat(div.style.height) - 3;
        if(n < 0)
            n = 0;
        div.style.height = n;
        if(n == 0) { // done closing
            div.style.display = "none";
            div.style.height = div.originalHeight;
            if(div.closeTimer) {
                clearInterval(div.closeTimer);
                div.closeTimer = 0;
            }
        }
    }
}
function toggleExpander(div, expander, expandText, collapseText) {
    if (div.style.display == "none") {
        div.originalHeight = parseFloat(div.style.height);
        //div.style.height = 0;
        div.style.display = "";
        expander.innerHTML = collapseText ? collapseText : defaultCollapseText;
        if(div.closeTimer) {
            clearInterval(div.closeTimer);
            div.closeTimer = 0;
        }
        //div.closeTimer = setInterval(function() { animateExpander(div, true); }, 1000);
    } else {
        div.style.display = "none";
        expander.innerHTML = expandText ? expandText : defaultExpandText;
        if(div.closeTimer) {
            clearInterval(div.closeTimer);
            div.closeTimer = 0;
        }
        //div.closeTimer = setInterval(function() { animateExpander(div, false); }, 1000);
    }  
}
function enableExpanders() {
   var divs = document.getElementsByTagName("div");
   for(var i = 0; i < divs.length; ++i) {
      var div = divs[i];
      if(div.className == "expanderChild") {
          var expander = div.parentNode.getElementsByTagName("span")[0];
          expander.style.display = "block";
          div.style.display = "none";
          div.style.overflow = "hidden";
          expander.innerHTML = defaultExpandText;
          var f = function() {
              var div2 = div;
              var expander2 = expander;
              expander.onclick = function() {
                 toggleExpander(div2, expander2);
              };
          };
          f();
      }
   }
}

// TODO maybe use <http://www.dynamicdrive.com/dynamicindex17/animatedcollapse.js>
