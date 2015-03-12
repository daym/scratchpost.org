<?php

if (!isset($topdir)) {
  $topdir = $_SERVER["DOCUMENT_ROOT"];
  if(substr($topdir, -1) != "/") {
    $topdir = $topdir . "/";
  }
}

error_reporting(E_ALL);
setlocale(LC_COLLATE, "en_US.UTF-8");
//$collator = collator_create("en_US");

$page_extra_files = array(); // user: set this to have the mtime checker check these files (also directories), too
if (file_exists("image")) { /* magic magic mushroom */
	array_push($page_extra_files, "image"); /* gallery mode is automatic if there exists a directory "image" */
	// preserve bandwidth by caching:
	header('Expires: ' . gmdate('D, d M Y H:i:s', time() + 24 * 60 * 60) . ' GMT');
}        

$real_script_url = $_SERVER['SCRIPT_NAME']; // without suffix path ("/some/script.php/more" => "/some/script.php")
$topuri = "/"; // TODO generize
/* file type heaven: http://www.wotsit.org/ */

function seealso_web($path)
{
  $see_also = array(); /* [(url, description)] */
  $f = @fopen($path . ".webseealso", "r");
  if ($f !== false) {
    while (($s = fgets($f, 2049)) !== false) {
      $s = trim($s);
      if (strlen($s) > 0 && substr($s,0,1) == "#") { /* comment */
        continue;
      }
      $a = split(" ", $s, 2);
      if (count($a) == 2) {
        array_push($see_also, $a);
      }
    }
    fclose($f);
  }
  return $see_also;
}

function drawurlencode($path) {
	return str_replace("+", "%20", escape($path));
}

function read_string_array_from_file($path)
{
  $f = @fopen($path, "r");
  $result = array();
  if ($f !== false) {
    while (($s = fgets($f, 2049)) !== false) {
      $s = trim($s);
      array_push($result, $s);
    }
    fclose($f);
  }
  
  return $result;
}

function load_string_from_file($path)
{
  $f = @fopen($path, "r");
  if ($f !== false) {
    $s = fgets($f, 2049);
    return $s;
  }
  
  return false;
}

function load_string_set_from_file($path)
{
  $f = @fopen($path, "r");
  $result = array();
  if ($f !== false) {
    while (($s = fgets($f, 2049)) !== false) {
      $s = trim($s);
      $result[$s] = 1;
    }
    fclose($f);
  }
  
  return $result;
}
function listdir_web_filtered($path)
{
  if ($path != "" && substr($path, -1) != "/") {
    $path = $path . "/";
  }

  $hidden = load_string_set_from_file($path . ".webhidden");

  $entries = array();
  // debug_print_backtrace();
  $opendir_path = $path;
  if ($opendir_path == "") {
    $opendir_path = getcwd();
  }
  $dir = opendir($opendir_path);
  while (($entry = readdir($dir)) !== false) {
    if (substr($entry, strlen($entry) - 8) == ".INC.PHP" 
    || isset($hidden[$entry]) 
    || substr($entry, 0, 1) == "."
    // || substr($entry, strlen($entry) - 1, 1) == "~"
    ) {
      continue;
    }
    array_push($entries, $entry);
  }
  closedir($dir);

  sort($entries);
  return $entries;

}

// takes a flat list and returns: dictionary-tree; key = display-text, value = (dictionary-tree|real-filename)
// @param flat_entries flat array of strings
// @param grouping (string) regular expression with groups, usually open-ended; can be "" which means no grouping
// @result dictionary-tree
function group_string_array($grouping, $flat_entries)
{
  $deep_nodes = array(); // key: $deep_key
  $deep_keys = array();
  
  $tree = array();
  
  $remainder = array();
  
  foreach ($flat_entries as $entry) {
    $matches = array();
    if ($grouping !== false && $grouping != "" && !ereg($grouping, $entry, $matches)) { // configurable
      $matches = false;
    } else {
      if ($grouping === false || $grouping == "") {
        $matches = array("");
      }
      $matched_entry = array_shift($matches);
      $short_entry = substr($entry, strlen($matched_entry));
    }
    
    if ($matches === false) { // unknown pattern
      $remainder[$entry] = $entry;
      continue;
    }
    
    while (count($deep_keys) < count($matches)) {
      array_push($deep_keys, false);
    }

    $deep_node = &$tree;
    
    for ($i = 0; $i < count($deep_keys); $i++) {
      $deep_key = $deep_keys[$i];
      $match = $matches[$i];

      if ($deep_key != $match) { // new group
        //print "new group" . $match;
        unset($deep_nodes[$deep_key]);
        $deep_key = $match;
        $deep_keys[$i] = $deep_key;
        $deep_node[$deep_key] = array(); 
        $deep_nodes[$deep_key] = &$deep_node[$deep_key]; // parent, map, link (not copy)
      } // end match != key
      
      $deep_node = &$deep_node[$deep_key];
    } // end for each deep key
    
    $deep_node[$short_entry] = $entry;
  } // end foreach flat_entries
  
  if (count($remainder) > 0) {
    $tree["unsorted"] = $remainder;
  }

  return $tree;
}

// returns: dictionary-tree; key = display-text, value = (dictionary-tree|real-filename)
// filters the self-reference out
function grouped_listdir_web_filtered($path)
{
  global $_SERVER;
  
  $flat_entries = listdir_web_filtered($path); // sorted

  $self_me = $_SERVER["PHP_SELF"];
  $base_self_me = basename($self_me);

  // = unset($flat_entries[value = basename($self_me)]); // if exists
  
  foreach ($flat_entries as $key => $value) {
    if ($value == $base_self_me) {
      unset($flat_entries[$key]);
    }
  }  

  $grouping_file = @fopen($path . "/.webgrouping", "r");
  if ($grouping_file === false) {
    $result = array();
    foreach ($flat_entries as $flat_entry) {
      $result[$flat_entry] = $flat_entry;
    }
    return $result;
  }
  
  $grouping = fgets($grouping_file, 2000);
  fclose($grouping_file);
  
  return group_string_array($grouping, $flat_entries);

}


function print_main_pathbar()
{
  $request_uri = $_SERVER["REQUEST_URI"];
  $request_uri_parts = explode("?", $request_uri . "?");
  $request_uri = $request_uri_parts[0]; // strip GET parameters
  
?>
<p><?php

$request_array = split("/", $request_uri);
$request_uri_so_far = "";
$first = true;
foreach ($request_array as $request_item) {

if ($request_item == "" && $first == true) {
} else {
  $request_uri_so_far = $request_uri_so_far . "/" . $request_item;
}

if ($request_item == "" && $first == true) {
  $request_item = "Home";
}

if (substr($request_uri, 0, 6) == "/wiki/") { /* why me... */
  if ($request_item == "index.php") { // stupid.
    continue;
  }
}

$first = false;

?> / <a href="<?= htmlentities($request_uri_so_far, ENT_COMPAT, "UTF-8") ?>/"><?= htmlentities(rawurldecode($request_item), ENT_COMPAT, "UTF-8") ?></a><?php
}

?></p>
<?php
}

function get_web_navigation_style()
{
  @$path = dirname($_SERVER["PATH_TRANSLATED"]) . "/";
  if (!$path) {
    $path = "";
  }
  
  $file = @fopen($path . ".web-navigation-style", "r");
  if ($file === false) {
    $navigation_style = "floating-left";
  } else {
    $navigation_style = fgets($file, 2049);
    $navigation_style = trim($navigation_style);
    fclose($file);
  }
  return $navigation_style;
}

function print_main_navigation($navigation_style = "floating-left")
{
  if ($navigation_style == "none") {
    return;
  }
  
  if ($navigation_style == "inline") {
    $class = "main_navigation_inline";
  } else {
    $class = "main_navigation";
  }
  
?>
<div class="<?= htmlentities($class, ENT_COMPAT, "UTF-8") ?>" id="main_navigation">
<?php
  print_main_pathbar();
  print_main_navigation_content();
?>
</div><!-- main_navigation -->
<?php
}

function unpack_I4($raw_data) {
	$numbers = unpack("L", $raw_data);
	return $numbers[1];
}

function get_file_type($path)
{
  $res = "application/octet-stream";
  if (is_dir($path)) {
    $res = "inode/directory";
  } else if (is_file($path)) {
    $f = @fopen($path, "rb");
    if ($f !== false) {
      $first_line = fread($f, 270);
      fclose($f);
      
      if (substr($first_line, 0, 5) == "<?php") {
        $res = "text/html"; // "probably"
      } else if (substr($first_line, 0, 5) == "<?xml") {
        $res = "application/xml";
        if (preg_match('/<svg/', $first_line)) {
          $res = "image/svg+xml";
        }
      } else if (substr($first_line, 0, 14) == "<!DOCTYPE html") {
        $res = "text/html";
      } else if (substr($first_line, 0, 4) == "\037\213\010\0" 
       || substr($first_line, 0, 4) == "\037\213\010\010") {
        $res = "application/x-gtar";
      } else if (substr($first_line, 257, 6) == "ustar\0" /* POSIX */
       || substr($first_line, 257, 8) == "ustar  \0"  /* GNU */) {
        $res = "application/x-tar";
      } else if (substr($first_line, 0, 6) == "\211PNG\r\n") {
        $res = "image/png";
      } else if (substr($first_line, 0, 4) == "\377\330\377\340" 
        || substr($first_line, 0, 4) == "\377\330\377\341") {
        $res = "image/jpeg";
      } else if (substr($first_line, 0, 2) == "BM" && unpack_I4(substr($first_line, 2, 4)) == filesize($path)) {
        $res = "image/x-ms-bmp"; // not exactly standard.
      } else if (substr($first_line, 0, 6) == "GIF89a") {
        $res = "image/gif";
      } else if (substr($first_line, 0, 4) == "RIFF" && substr($first_line, 8, 4) == "AVI ") {
        // && [7] < 32 (high bit filesize) ?
        $res = "video/x-msvideo";
      } else if (substr($first_line, 0, 21) == "!<arch>\ndebian-binary") {
        $res = "application/x-debian-package";
      } else if (substr($first_line, 0, 13) == "%!PS-Adobe-2.") {
        $res = "application/postscript";
      } else if (substr($first_line, 0, 5) == "%PDF-") {
        $res = "application/pdf";
      } else if (substr($first_line, 0, 4) == "PK\3\4") { // \024 or \012
        $res = "application/zip";
      } else if (substr($first_line, 0, 4) == "\312\376\272\276") {
        $res = "application/x-java-class";
      } else if (substr($first_line, 0, 2) == "# " || substr($first_line, 0, 2) == "#!") {
        $res = "text/plain"; // or probably some script, actually.
      } else if (substr($first_line, 0, 7) == "Index: ") {
        $res = "text/plain"; // or probably some patch.
      } else if (substr($first_line, 0, 4) == "ID3\3") {
        $res = "audio/mpeg";
      } else if (substr($first_line, 0, 9) == "\060\046\262\165\216\146\317\021\246") { // \246\311") { // \0\252\0\142\316\154") {
        $res = "audio/x-ms-wma";
      } else if (substr($first_line, 10, 4) == "\377\377\377\377") { // HACK HACK
          $res = "image/svg+xml";
      } else if (!preg_match('/[\0-\7\16-\37]/', $first_line)) { // TODO allow ANSI escapes?
         // '/[\0-\37]+/', $first_line)) {
        $res = "text/plain"; 
      } else if(substr($first_line, 0, 5) == "\177ELF\1") {
        $res = "application/x-executable";
      } else if(substr($first_line, 0, 4) == "MZ\220\0") {
        $res = "application/x-executable";
      } else if(substr($first_line, 0, 5) == "\000\001\000\000\000") { // FIXME "head".
        $tag = substr($first_line, 12, 4); // FFTM
        $res = "application/vnd.font-opentype"; // FIXME
      }
    }
  } else {
    $res = false;
  }
  return $res;
}

function recursive_private_foreach_entry_listdir_web($entries, $group_format_strings, $level)
{
  if ($level >= 1000) { // just in case 
    return;
  }
  
  asort($entries, SORT_LOCALE_STRING);
  $entry_keys = array_keys($entries);
  foreach($entry_keys as $entry_key) {
    $entry = $entries[$entry_key];
    if (is_array($entry)) {
      if (isset($group_format_strings[$level])) {
        $format_string = $group_format_strings[$level];
      } else {
        $format_string = "%s";
      }
      
      $title = sprintf($format_string, $entry_key);
?>
<li class="listdir_web_sub_category"><h2><?= htmlspecialchars($title) ?></h2><ul>
<?php
      recursive_private_foreach_entry_listdir_web($entry, $group_format_strings, $level + 1);
?>
</ul></li>
<?php
      continue;
    }
    
    $extrainfo = "";
  
    $file_type = get_file_type($entry);
  
    $icon_name = "unknown";
    if ($file_type == "inode/directory") {
      $icon_name = "folder";
    } else if ($file_type == "text/html") {
      $icon_name = "page";
    } else if ($file_type == "application/x-gtar") {
      $icon_name = "tarball";
    } else if ($file_type == "application/x-tar") {
      $icon_name = "tarball";
    } else if ($file_type == "application/zip") {
      $icon_name = "zip";
    } else if ($file_type == "application/x-executable") {
      $icon_name = "executable";
    } else if ($file_type == "application/x-debian-package") {
      $icon_name = "debian_package";
    } else if ($file_type == "application/x-java-class") {
      $icon_name = "bytecode";
    } else if (substr($file_type, 0, 6) == "image/") {
      $icon_name = "image";
    } else if (substr($file_type, 0, 6) == "video/") {
      $icon_name = "video";
    } else if ($file_type == "application/postscript") {
      $icon_name = "printable_document";
    } else if ($file_type == "application/pdf") {
      $icon_name = "printable_document";
    } else if ($file_type == "text/plain") {
      $icon_name = "plain_text";
    } else if (substr($file_type, 0, 6) == "audio/") {
      $icon_name = "audio";
    }
  
    $the_filesize = @filesize($entry);
    if ($the_filesize !== false && $the_filesize > 50000) {
      $extrainfo = "&nbsp;(" . round($the_filesize / 1024) . "&nbsp;KiB)";
    }
    
    $type_suffix = "";
    if ($file_type == "inode/directory") {
      $type_suffix = "/";
    }

?>
<li class="type_<?= $icon_name ?>">&nbsp;<a href="<?= htmlentities(str_replace("+", "%20", urlencode($entry)), ENT_COMPAT, "UTF-8") ?>" type="<?= htmlentities($file_type) ?>"><?= htmlentities($entry_key . $type_suffix, ENT_COMPAT, "UTF-8") . $extrainfo ?></a></li>
<?php

  } /* foreach listdir_web_filtered */
}

function print_main_navigation_content()
{
  global $topdir;
  $request_uri = $_SERVER["REQUEST_URI"];
?>
<ul class="main_directory">
<?php
  if (substr($request_uri, 0, 6) == "/wiki/") { /* why me... */
    if ($request_uri == "/wiki/index.php/Main_Page") {
?>
<li><a href="/" type="inode/directory">[Go Up]</a></li>
<?php
    } else {
?>
<li><a href="/wiki/index.php/Main_Page" type="inode/directory">[Go Up]</a></li>
<?php
    } /* end wiki main page */
  } else {
  /*<img src="/common/icon/chapter.PNG" alt="" width="15" height="13"/>*/
?>
<li class="type_folder">&nbsp;<a href=".." type="inode/directory">[Go Up]</a></li>
<?php
  } /* end if substr $request_uri /wiki/ */

$URL_parts = explode("#", $request_uri, 2);
$resulting_path = $topdir . urldecode($URL_parts[0]);

if(get_file_type($resulting_path) == "inode/directory") {
  $entries = grouped_listdir_web_filtered("");
} else { /* a file */
  $entries = array();
}

// TODO maybe if the page is "index.php" and that is the current page, filter it out.

$group_format_strings = read_string_array_from_file(".webgrouping-formats");


recursive_private_foreach_entry_listdir_web($entries, $group_format_strings, 0);

$see_alsos = seealso_web("");
foreach($see_alsos as $see_also) {
  $url = $see_also[0];
  $description = $see_also[1];
  
?>
<li class="type_symbolic_link"><a href="<?= htmlspecialchars($url) ?>">See also: <?= htmlspecialchars($description) ?></a></li>
<?php
} /* foreach see_alsos */

?></ul>
<?php
}

$page_header_CWD = false;

function print_page_header($dir_id = false, $title = false, $head_cb = false, $body_cb = false)
{
  global $page_header_CWD;
  global $body_attribute_adds;
  global $_SERVER;
  global $topdir;
  global $topuri;

  $page_header_CWD = getcwd(); // workaround PHP weirdness where it chdir's to "/" before calling functions registered with "register_"*

  if (!isset($topuri)) {
    $topuri = "/"; // $topdir;
  }

  if (!$dir_id) {
    $dir_id = $_SERVER["REQUEST_URI"];
  }

  if (!$title) {
    $title = rawurldecode($dir_id);
    $parts = explode("/", $title);
    if (count($parts) > 0) {
      if ($parts[count($parts) - 1] == "") {
        array_pop($parts);
      }
      if (count($parts) > 0) {
        $title = str_replace("_", " ", $parts[count($parts) - 1]);
      }
    }
  }
  
  $request_uri = $_SERVER["REQUEST_URI"];
  print '<?xml version="1.0" encoding="utf-8"?>' . "\n";
  register_shutdown_function("print_page_footer");
/*
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
*/
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="EN">
<head>
<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=UTF-8"/><!-- IE -->
<title><?= htmlspecialchars(urldecode($dir_id)) ?></title>
<?php
if ($head_cb === false || $head_cb() == false) {
?>
<meta name="MSSmartTagsPreventParsing" content="TRUE"/>
<link rel="openid.server" href="http://www.livejournal.com/" />
<link rel="openid.delegate" href="http://dannymi.livejournal.com/" />
<link rel="stylesheet" href="<?= $topuri ?>common/scratchpost.css" type="text/css"/>
<link rel="shortcut icon" href="<?= $topuri ?>common/icon/scratchpost.PNG" type="image/png"/>
<link rel="icon" href="<?= $topuri ?>common/icon/scratchpost.PNG" type="image/png"/>
<link rel="up" href="../"/>
<script type="text/javascript" src="<?= $topuri ?>common/scratchpost.js">
</script>
<!-- gallery: TODO: make optional -->
<script type="text/javascript" src="/common/scaler.js">
</script>
<?php
}
?>
</head>
<body <?= $body_attribute_adds ?>>
<?php
if ($body_cb === false || $body_cb() == false) {
}
?>
<table border="0" class="main_navigation_area"><tr><td><?php

	$navigation_style = get_web_navigation_style();
	print_main_navigation($navigation_style);

	if ($navigation_style == "inline") {
		$border_class = "main_border_inline";
	} else {
		$border_class = "main_border";
?>
</td><td class="main">
<?php
	}

	if ($navigation_style != "none") { // TODO extra option?
?>
<h1><?= htmlspecialchars($title) ?></h1>
<?php
	}
?>
<div class="<?= htmlentities($border_class, ENT_COMPAT, "UTF-8") ?>" id="main_border"><div class="main_content">
<?php

	if (file_exists("image") && !file_exists("image/equation")) { /* magic magic mushroom */
		require_once($topdir . "common/gallery.INC.PHP");
		print_image_gallery();
	}
}

function get_title_of_href($href)
{
  $result = $href;
  if (substr($href, -4) == ".PHP") {
    $result = substr($href, 0, strlen($href) - 4);
  }
  
  if (substr($result, 0, 2) == "./") {
    $result = substr($result, 2);
  }
  return $result;
}

function get_newest_subpage($base, &$mtime, &$modified_location_href, &$modified_location_title)
{
  $subfiles = listdir_web_filtered($base);
  if ($subfiles !== false) {
    foreach ($subfiles as $sub_file_name) {
      $sub_file_path = $base . $sub_file_name; // .
      
      $n_mtime = filemtime($sub_file_path);
      if ($n_mtime > $mtime) {
        $mtime = $n_mtime;
        
        if (is_file($sub_file_path)) { /* file */
          $modified_location_href = $sub_file_path;
        } else if (is_dir($sub_file_path)) { /* directory */
          $modified_location_href = $sub_file_path . "/";
        } else {
          $modified_location_href = $sub_file_path;
        }

        $modified_location_title = get_title_of_href($modified_location_href);
      }
      
      if (is_dir($sub_file_path)) { /* directory */
        get_newest_subpage($sub_file_path . "/", $mtime, $modified_location_href, $modified_location_title);
      }
    }
  }
  
  /* todo recurse into directories to see if there is any newer? probably overkill */
}

function print_page_mtime($directory)
{
  global $page_extra_files;
  
  //$time_format = "%Y-%m-%d %H:%M";
  $mtime = filemtime($_SERVER["SCRIPT_FILENAME"]) - date("Z");

  if (isset($page_extra_files)) { 
    foreach($page_extra_files as $file_path) {
      $n_mtime = @filemtime($file_path);
      if ($n_mtime !== false && $n_mtime > $mtime) {
        $mtime = $n_mtime;
      }
    }
  }
  
  $modified_location_href = false; // TODO maybe use the "." dir href.
  $modified_location_title = false; // TODO maybe use the current page title.
  $mtime = filemtime(".");
  get_newest_subpage($directory, $mtime, $modified_location_href, $modified_location_title);
  
  
  
  $gmt_mtime = date('D, d M Y H:i:s', $mtime) . ' GMT';
  
  $gmt_mtime_date = date("D, d M Y", $mtime);
  
?>
<p>Last modification on: <span title="<?= $gmt_mtime ?>"><?= $gmt_mtime_date ?></span>
<?php
  if ($modified_location_href != false) {
?> in <a href="<?= htmlspecialchars($modified_location_href) ?>"><?= htmlspecialchars($modified_location_title) ?></a><?php
  } /* $modified_location */
?>.
</p>
<?php
}

$printed_page_footer = false;

function print_page_footer($dir_id = false /* uses default. You can usually call this without argument. */)
{
  global $printed_page_footer;
  global $page_header_CWD;
  //global $navigation_style;
  
  if ($printed_page_footer) {
    return;
  }
  chdir($page_header_CWD);

  $printed_page_footer = true;
  $navigation_style = get_web_navigation_style();
  if ($navigation_style != "none") {
    $author_HTML = load_string_from_file(".webauthor");
?>
<p>Author: 
<?php
    if($author_HTML !== false) {
      print $author_HTML;
    } else {
?>
<a href="mailto:danny.milo@gmail.com.nospam">Danny</a> (remove the ".nospam" to send)
<?php
    }
?>
</p>
<?php
print_page_mtime("");
  }
?>
<!--<p>&nbsp;</p>-->
</div><!-- class main_content -->
</div><!-- class main_border -->
</td></tr></table><!-- main_navigation_area -->
</body>
</html>
<?php
}

function get_random_saying()
{
  global $topdir;
  $entries = listdir_web_filtered($topdir . "saying");
  $index = rand(0, count($entries) - 1);
  
  $file_path = $topdir . "saying/" . $entries[$index];
  $f = fopen($file_path, "r");
  $res = "";
  while (($s = fgets($f, 2049)) !== false) {
    $res = $res . $s;
  }
  fclose($f);
  return $res;
}

// similar services exist for other IMs, for example PingBox for Yahoo.
$ICQ_UIN = "575166204";
function print_ICQ_online_status() {
?>
<img src='http://status.icq.com/online.gif?icq=<?= $ICQ_UIN ?>&amp;img=5' border='0' width='18' height='18' align='absmiddle'/>
<?php
}

function print_ICQ_pager() {
/* TODO do this is a floating DIV? */
	print_ICQ_online_status();
?>
<form action="http://msg.mirabilis.com/scripts/WWPMsg.dll" method="POST" name="frmPager">
<input type="hidden" name="subject" value="From WebPager Panel"/>
<input type="hidden"  name="to" value="<?= $ICQ_UIN ?>"/>
<label for="ICQ_from">Your Name: </label>
<input type="text" id="ICQ_from" name="from" value="" size="15" maxlength="40" />
<label for="ICQ_fromemail">Your E-Mail: </label>
<input type="text" name="fromemail" id="ICQ_fromemail" value="" size="15" maxlength="40" />
<label for="ICQ_body">Your Text: </label>
<input name="body" id="ICQ_body" type="text" size="32" maxlength="400" value=""/>
<input type="hidden" name="range" value="web">
<input type="hidden" name="up" value="cp2">
<input type="image"/>
</form>

<?php
}

?>