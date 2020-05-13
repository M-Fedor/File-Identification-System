<?php
define('DEFAULT_ROW_ELEMENT_COUNT', 12);
define('FILE_ROW_ELEMENT_COUNT', 15);

class ContentGenerator
{
    function __construct() 
    { 
        $this->counts['error'] = $this->counts['suspicious'] = 
            $this->counts['valid'] = $this->counts['warning'] = 0;
        $this->is_success = True;
        $this->result_content = ''; 
    }

    function generate_bad_request() 
    {
        $this->result_content .= "<p class=\"alert\">Request is invalid. Some required fields are not filled.</p>";
        $this->is_success = False;
    }

    function generate_content()
    {
        include(BASE_PATH_PREFIX . 'frontend/head.html');

        echo "<div id=\"wrapper_out\">\n<div id=\"wrapper\">\n";

        include(BASE_PATH_PREFIX . 'frontend/header.html');

        echo "<div id=\"content\">";

        require(BASE_PATH_PREFIX . 'frontend/basic_content.html');

        echo "<div id=\"result\">";

        $this->generate_result();

        echo "</div>\n</div>\n</div>\n</div>\n\n";
        
        include(BASE_PATH_PREFIX . 'frontend/assets/graphics.html');
        echo "<script src=\"/frontend/functions/functions.js\"></script>\n";
    }

    function generate_result_file(&$result)
    {
        $rows_count = count($result);

        for($i = 0; $i < $rows_count; $i++)
        {
            $row_length = count($result[$i]);
            $file_name = $this->get_file_name($result[$i][1]);

            if ($row_length == FILE_ROW_ELEMENT_COUNT)
            {
                $status = $result[$i][0];
                $this->result_content .= "\t<tr class=\"$status\">\n<td>$file_name</td>\n";

                for ($j = 1; $j < $row_length; $j++)
                    $this->result_content .= "\t\t<td>" . $result[$i][$j] . "</td>\n";
                 $this->result_content .= "\t</tr>";
            }
            else
            {
                $status = 'error';
                $this->result_content .= "\t<tr class=\"$status\">\n<td>$file_name</td>\n
                    <td colspan=14>Found result has incorrect format. Either filename contains \",\" or you're just trying us.</td>\n";
            }

            $this->counts[$status]++;
        }
    }

    function generate_result_mysql(&$original_digest, &$original_path, &$result)
    {
        $rows_count = $result->num_rows;
        $file_name = $this->get_file_name($original_path);

        if ($rows_count == 0)
            return $this->generate_result_unknown($file_name, $original_digest, $original_path);

        // Determine actual length of result rows
        $row_length = count($result->fetch_row());
        $result->data_seek(0);

        for ($i = 0; $i < $rows_count; $i++)
        {
            $row = $result->fetch_row();
            $status = ($original_path == $row[0]) ? 'valid' : 'warning';
            $status = ($original_digest != $row[1]) ? 'suspicious' : $status;
            $counts[$status]++;

            $this->result_content .=  
                "\t<tr class=\"$status\">\n\t\t<td>$file_name</td><td>$original_path</td>\n\t\t<td>$original_digest</td>\n";

            for ($j = 0; $j < $row_length; $j++)
                $this->result_content .= "\t\t<td>" . $row[$j] . "</td>\n";
            $this->result_content .= "\t</tr>";
        }
    }

    function generate_server_error() 
    {
        $this->result_content .= "<p class=\"alert\">It's not you, it's us. System error occurred on server side.</p>";
        $this->is_success = False;
    }

    private function generate_result()
    {
        if ($this->result_content == '')
            return;
        if ($this->is_success)
            $this->generate_success_result();
        else
            echo $this->result_content;
    }

    private function generate_success_result() 
    {
        echo "<p class=\"info\">All files have been checked successfully. " . $this->counts['valid'] . " valid files, " . $this->counts['suspicious'] . 
            " suspisions, " . $this->counts['warning'] . " warnings and " . $this->counts['error'] ." errors were found.</p>";

        include(BASE_PATH_PREFIX . 'frontend/filters.html');

        echo "<div id=\"data\">\n<div class=\"overflow_content\">\n<table>\n";

        echo $this->table_head . $this->result_content;

        echo "</table>\n</div>\n</div>\n\n";
    }

    private function generate_result_unknown(&$file_name, &$digest, &$path_name)
    {
        $this->result_content .=    
            "\t<tr class=\"unknown\">\n\t\t<td>$file_name</td><td>$path_name</td>\n\t\t<td>$digest</td>\n";

        for ($i = 0; $i < DEFAULT_ROW_ELEMENT_COUNT; $i++)
            $this->result_content .= "\t\t<td>Unknown</td>\n";
        $this->result_content .= "\t</tr>";
    }

    private function get_file_name(&$file_path)
    {
        if ($file_path == NULL)
            return '';

        $pos = max(strrpos($file_path, '/'), strrpos($file_path, '\\'));
        if ($pos === False)
            return $file_path;
        return substr($file_path, $pos + 1);
    }

    private $counts;
    private $is_success;
    private $result_content;

    private $table_head = "<thead>\n\n<tr>\n<th>File name</th>\n<th>Original path</th>\n<th>Original digest</th>\n<th>File path</th>\n
        <th>File digest</th>\n<th>Created</th>\n<th>Modified</th>\n<th>Registered</th>\n<th>File type</th>\n<th>Company name</th>\n
        <th>Product name</th>\n<th>Product version</th>\n<th>File version</th>\n<th>File description</th>\n<th>OS version</th>\n</tr>\n</thead>";
}
?>
