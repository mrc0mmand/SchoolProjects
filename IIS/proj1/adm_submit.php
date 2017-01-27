<?php

session_start();

include "db.php";

$method = "";

if(isset($_GET["dropType"])) {
    switch($_GET["dropType"]) {
    case "user":
        $method = "list-users";
        $stmt = $mysqli->prepare("
            DELETE FROM users
            WHERE id = ?
            AND username <> ?
        ");
        $stmt->bind_param("is", $_GET["userID"], $_SESSION["username"]);
        if(!$stmt->execute()) {
            $_SESSION["lasterror"] = "SQL error: " . $mysqli->error;
        }

        break;
    default:
        $_SESSION["lasterror"] = "Invalid operation";
    }
} else {
    $_SESSION["lasterror"] = "Invalid operation";
}

if($method) {
    header("Location: is.php?method=" . $method);
} else {
    header("Location: is.php");
}

?>
