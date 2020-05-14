<?php
define('BASE_PATH_PREFIX', '');
require('backend/ContentGenerator.php');

$gen = new ContentGenerator();
$gen->generate_content();
?>
