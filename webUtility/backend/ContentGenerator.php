<?php
define('DEFAULT_ROW_ELEMENT_COUNT', 12);

class ContentGenerator
{
    function __construct() 
    { 
        $this->isSuccess = True;
        $this->result_content = ''; 
    }

    function generate_bad_request() 
    {
        $this->result_content .= file_get_contents(BASE_PATH_PREFIX . 'frontend/bad_request.html');
        $this->isSuccess = False;
    }

    function generate_content()
    {
        $this->generate_header();
        $this->generate_body();
        $this->generate_result();
        $this->generate_footer();
    }

    function generate_result_file(&$result)
    {
        $rows_count = count($result);
        $row_length = count($result[0]);

        for($i = 0; $i < $rows_count; $i++)
        {
            $status = $result[$i][0];
            $this->result_content .= "\t<tr class=\"$status\">\n";

            for ($j = 1; $j < $row_length; $j++)
                $this->result_content .= "\t\t<td>" . $result[$i][$j] . "</td>\n";
                 
            $this->result_content .= "\t</tr>";
        }
    }

    function generate_result_mysql(&$original_digest, &$original_path, &$result)
    {
        $rows_count = $result->num_rows;

        if ($rows_count == 0)
        {
            $this->result_content .=    "\t<tr class=\"unknown\">\n
                                            \t\t<td>$original_path</td>\n
                                            \t\t<td>$original_digest</td>\n";

            for ($i = 0; $i < DEFAULT_ROW_ELEMENT_COUNT; $i++)
                $this->result_content .= "\t\t<td></td>\n";
            $this->result_content .= "\t</tr>";
            return;
        }

        // Determine actual length of result rows
        $row_length = count($result->fetch_row());
        $result->data_seek(0);

        for ($i = 0; $i < $rows_count; $i++)
        {
            $row = $result->fetch_row();
            $status = ($original_path == $row[0]) ? 'valid' : 'warning';
            $status = ($original_digest != $row[1]) ? 'suspicious' : $status;

            $this->result_content .=    "\t<tr class=\"$status\">\n
                                            \t\t<td>$original_path</td>\n
                                            \t\t<td>$original_digest</td>\n";

            for ($j = 0; $j < $row_length; $j++)
                $this->result_content .= "\t\t<td>" . $row[$j] . "</td>\n";
            $this->result_content .= "\t</tr>";
        }
    }

    function generate_server_error() 
    {
        $this->result_content .= file_get_contents(BASE_PATH_PREFIX . 'frontend/server_error.html');
        $this->isSuccess = False;
    }

    private function generate_body() { require(BASE_PATH_PREFIX . 'frontend/body.html'); } 

    private function generate_footer() { include(BASE_PATH_PREFIX . 'frontend/footer.html'); }

    private function generate_header() { include(BASE_PATH_PREFIX . 'frontend/header.html'); }

    private function generate_result() 
    {
        if ($this->result_content == '')
            return;

        if (!$this->isSuccess)
            echo $this->result_content;
        else
            echo    "<table>\n
                        \t<thead>\n
                            \t\t<tr>\n
                                \t\t\t<th>Original path</th>\n
                                \t\t\t<th>Original digest</th>\n
                                \t\t\t<th>File path</th>\n
                                \t\t\t<th>File digest</th>\n
                                \t\t\t<th>Created</th>\n
                                \t\t\t<th>Modified</th>\n
                                \t\t\t<th>Registered</th>\n
                                \t\t\t<th>File type</th>\n
                                \t\t\t<th>Company name</th>\n
                                \t\t\t<th>Product name</th>\n
                                \t\t\t<th>Original path</th>\n
                                \t\t\t<th>Product version</th>\n
                                \t\t\t<th>File version</th>\n
                                \t\t\t<th>File description</th>\n
                                \t\t\t<th>OS version</th>\n
                            \t\t</tr>\n
                        \t</thead>\n
                        $this->result_content
                        </table>\n";
    }

    private $isSuccess;
    private $result_content;
}
?>
