<?php

if (!isset($show_all_images_completely)) {
	$show_all_images_completely = true;
}

function escape($x)
{
	return str_replace("%2F", "/", urlencode($x));
}

function display_image_thumbnail_link($entry)
{
  global $topuri;
  global $_SERVER;
  global $real_script_url;
  global $_GET;
  global $show_all_images_completely;

  if (isset($_GET["select_image"])) {
    $current_image_basename = $_GET["select_image"];
  } else {
    $current_image_basename = "";
  }
    
  $request_uri = $_SERVER["REQUEST_URI"];
  //$real_script_url;
  $request_uri_parts = explode("/", $request_uri);
  array_pop($request_uri_parts);
  $base_directory_name = urldecode(implode("/", $request_uri_parts)) . "/image/";

// escape($real_script_url) .   
if ($current_image_basename != $entry && !$show_all_images_completely) {
?><a id="anchor_<?= str_replace("=", "", base64_encode($entry)) ?>" class="image_of_the_day_thumbnail" href="<?= htmlspecialchars("?select_image=" . escape($entry)) ?>#anchor_<?= str_replace("=", "", base64_encode($entry)) ?>"
  ><img class="thumbnail" src="<?= $topuri ?>common/thumbnail/<?= htmlspecialchars(escape($base_directory_name . $entry)) ?>"
 alt="<?= htmlspecialchars($entry) ?>" title="<?= htmlspecialchars($entry) ?>" width="120" height="104" border="3"
/></a>
<?php
} else {
?><div id="anchor_<?= str_replace("=", "", base64_encode($entry)) ?>" class="image_of_the_day_big_image">
<p><?= htmlspecialchars(str_replace("_", " ", $entry)) ?> <a href="#" onclick="return resize_to_original_size(event)">Original Size</a> | <a href="#" onclick="return fit_to_window_event(event, true)">Fit to Window Width</a></p>
<?php

global $topdir;

if(get_file_type($topdir . $base_directory_name . $entry) == "image/svg+xml") {
?>
<object type="image/svg+xml" data="<?= htmlspecialchars(drawurlencode($base_directory_name . $entry)) ?>" >?</object>
<?php
} else {
?>
<img src="<?= htmlspecialchars(drawurlencode($base_directory_name . $entry)) ?>" alt="<?= htmlspecialchars($entry) ?>" 
title="<?= htmlspecialchars($entry) ?>"
 class="page" />
<?php
}
?>
</div>
<?php
}
?>
<?php
}

function display_image_group($entries, $group_formats, $level)
{
  if ($level >= 1000) {
    return;
  }

  $keys = array_keys($entries);

  // while this looks strange, firefox doesn't line break <ul>s, so we have to fake the last level (i.e. the leaves cannot be <li>s).

  $is_list = false;

  if (is_array($entries[$keys[0]])) { // probably a group
    $is_list = true; 
?>
<ul class="image_of_the_day">
<?php
  }

?>
<?php

  foreach ($keys as $key) {
    $entry = $entries[$key];

?>
<?php
    if (is_array($entry)) {
      if (isset($group_formats[$level])) {
        $format_string = $group_formats[$level];
      } else {
        $format_string = "%s";
      }
      $title = sprintf($format_string, $key);

?><li>
<h2><?= htmlspecialchars($title) ?></h2><ul class="image_helper">
<?php
      display_image_group($entry, $group_formats, $level + 1);
?>
</ul></li>
<?php
    } else {
      display_image_thumbnail_link($entry);
    }
?>
<?php
  }

  if ($is_list) {
?>
</ul>
<?php
  }
?>
<br clear="both" />
<?php
}


function print_image_gallery()
{
	// get file list   
	$flat_entries = listdir_web_filtered("image");
	if(count($flat_entries) == 0)
		return;

	// limit to the last 80 entries
	// TODO either handle or skip directories

	$current = array_pop($flat_entries);
	array_push($flat_entries, $current);
 
	while (count($flat_entries) > 80) {
		array_shift($flat_entries);
	}
   
	// group by year and month
	
	$group_format_strings = read_string_array_from_file("image/.webgrouping-formats");
	//print_r($group_format_strings);print "STR";

	$grouping_file = @fopen("image/.webgrouping", "r");
	if ($grouping_file === false) {
		$grouping = "";
	} else {
		$grouping = fgets($grouping_file, 2000);
		fclose($grouping_file);
	}
                                
	//$grouping = "^(....)(..)";
	$entries = group_string_array($grouping, $flat_entries);
   
	// $group_formats = array("Year %s", "Month %s");

	display_image_group($entries, $group_format_strings, 0);
?>
<br clear="both"/>
<?php
   
}

?>