<?php

session_start();
if(!isset($_SESSION["loggedin"])) {
    header("Location: index.php");
}

if(isset($_GET["logout"])) {
    unset($_SESSION["loggedin"]);
    session_unset();
    session_destroy();
    header("Location: index.php");
    exit;
}

?>
