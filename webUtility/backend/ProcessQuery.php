<?php
define('BASE_PATH_PREFIX', '../');
require('ContentGenerator.php');
require('DBConnection.php');

$gen = new ContentGenerator();

function process_file() 
{
    GLOBAL $gen;
    $conn = new DBConnection('localhost', 'user_name', 'password', 'test');
    if (!$conn->init()) 
        return $gen->generate_server_error();

    $file_name = $_FILES['file']['tmp_name'];
    $original_path = (isset($_POST['file_path'])) ? $_POST['file_path'] : '';
    if ($original_path == '') 
        return $gen->generate_bad_request();

    $fd = fopen($file_name, 'r');
    if (!$fd)
        return $gen->generate_server_error();

    $digest = strtoupper(hash_file('sha256', $file_name, FALSE));

    if (!$conn->execute_select($digest, $original_path))
        return $gen->generate_server_error();
   
    fclose($fd);
    $gen->generate_result_mysql($digest, $original_path, $conn->result);
}

function process_hash_file()
{
    GLOBAL $gen;
    $conn = new DBConnection('localhost', 'user_name', 'password', 'test');
    if (!$conn->init())
        return $gen->generate_server_error();

    $file_name = $_FILES['hash_file']['tmp_name'];

    $fd = fopen($file_name, 'r');
    if (!$fd)
        return $gen->generate_server_error();

    while (True)
    {
        $path_name = fgets($fd);
        $digest = fgets($fd);
        if (!$path_name || !$digest)
            break;

        if (!$conn->execute_select($digest, $path_name))
            return $gen->generate_server_error();
        $gen->generate_result_mysql($digest, $path_name, $conn->result);
    }   

    fclose($fd);
}

function process_result_file() 
{
    GLOBAL $gen;
    $file_name = $_FILES['result_file']['tmp_name'];

    $fd = fopen($file_name, 'r');
    if (!$fd)
        return $gen->generate_server_error();

    $result = [];
    while ($line = fgetcsv($fd, 0))
        array_push($result, $line);

    fclose($fd);
    $gen->generate_result_file($result);
}

if (!empty($_FILES['file']['tmp_name'])) 
    process_file();
else if (!empty($_FILES['hash_file']['tmp_name']))
    process_hash_file();
else if (!empty($_FILES['result_file']['tmp_name']))
    process_result_file();

$gen->generate_content();
?>
