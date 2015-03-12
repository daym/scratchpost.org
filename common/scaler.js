/* javascript */

function resize_to_original_size(e)
{
  var event = e || window.event;
  var source = event.target || event.srcElement;
  var original_image = source.parentNode.parentNode.getElementsByTagName("img")[0];
  //var original_image = document.getElementById("current_image");

  var original_width = 0;
  var original_height = 0;

  if (original_image.original_width == null) {
    return false;
  }

  new_width = original_image.original_width;
  new_height = original_image.original_height;
  original_image.width = new_width;
  original_image.height = new_height;
  return false;
}

function get_inner_width()
{
//  if (self.innerWidth != null) {
//    return self.innerWidth;
//  } else 
  if (document.body != null && document.body.clientWidth != null) {
    return document.body.clientWidth;
  } else {
    return 0; /* 744 */
  }
}   

function get_inner_height()
{
  if (self.innerHeight != null) {
    return self.innerHeight;
  } else if (document.body != null && document.body.clientHeight != null) {
    return document.body.clientHeight;
  } else {
    return 0; /* 400 */
  }
}

function get_root_left(item)
{
  var left = 0;
  while (item != null) {
    left = left + item.offsetLeft;
    item = item.offsetParent;
  }

  return left;

}

function get_root_top(item)
{
  var top = 0;

  while (item != null) {
    top = top + item.offsetTop;
    item = item.offsetParent;
  }

  return top;
}


function fit_to_window(original_image, deep_within_the_page)
{
  var original_width = 0;
  var original_height = 0;

  if (original_image.original_width == null) {
    original_image.original_width = original_image.width;
    original_image.original_height = original_image.height; 
  }

  original_width = original_image.original_width;
  original_height = original_image.original_height;

  var dest_width = get_inner_width();
  var dest_height = get_inner_height();

  if (dest_width == 0 || dest_height == 0) {
    return;
  }

  dest_width = dest_width - 15 * 2
   - original_image.style.borderWidth * 2 
   - original_image.style.paddingLeft 
   - original_image.style.paddingRight;

  /*dest_height = dest_height - 105 
   - original_image.style.borderWidth * 2
   - original_image.style.paddingTop
   - original_image.style.paddingBottom;*/

  dest_width = dest_width - get_root_left(original_image);
  /*if (!deep_within_the_page) {
    dest_height = dest_height - get_root_top(original_image);
  }*/

  aspect_ratio = original_width / original_height;
  dest_height = dest_width / aspect_ratio;

  if (original_width < dest_width && original_height < dest_height) {
    /* smaller: leave it alone */
    return;
  }

  if (aspect_ratio < (dest_width / dest_height)) {
    new_width = dest_height * aspect_ratio;
    new_height = dest_height;
  } else {
    new_height = dest_width / aspect_ratio;
    new_width = dest_width;
  }

  original_image.width = new_width;
  original_image.height = new_height;
}

function fit_to_window_event(e, deep_within_the_page)
{
  var event = e || window.event;
  var source = event.target || event.srcElement;
  var original_image = source.parentNode.parentNode.getElementsByTagName("img")[0];
  // assert original_image.class == "image_of_the_day_current"
  fit_to_window(original_image, deep_within_the_page);
  return false;
}

function autoscale_images(deep_within_the_page)
{ 
  var original_images = document.getElementsByTagName("img");
  for(var i = 0; i < original_images.length; ++i) {
    var original_image = original_images[i];
    if (original_image.getAttribute("class") == "image_of_the_day_current") {
      fit_to_window(original_image, deep_within_the_page);
    }
  }
  var original_images = document.getElementsByTagName("object"); // TODO filter non-SVGs?
  for(var i = 0; i < original_images.length; ++i) {
    var original_image = original_images[i];
    original_image.original_width = 800; // hello chrome
    original_image.original_height = 1131; // hello chrome
    // TODO getSVGDocument only
    //if (original_image.getAttribute("class") == "image_of_the_day_current") {
      fit_to_window(original_image, deep_within_the_page);
    //}
  }
  return true;
}


