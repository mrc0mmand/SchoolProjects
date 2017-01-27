<?php

function session_timeout() {
    # Session timeout: 10 minutes
    $SESSION_TIMEOUT = 600;

    if(!isset($_SESSION["lastact"])) {
        return;
    }

    if($_SESSION["lastact"] + $SESSION_TIMEOUT < time()) {
        header("Location: logout.php?logout");
    }
}

function hash_pass($pass) {
    return hash("sha256", $pass);
}

function user_login($user, $pass, &$error, &$level = NULL) {
    global $mysqli;
    $reason = "";

    $stmt = $mysqli->prepare("SELECT * FROM users WHERE username = ?");
    if($stmt) {
        $stmt->bind_param("s", $user);
        $stmt->execute();
        $res = $stmt->get_result();
        $row = $res->fetch_assoc();
        if($row["password"] === $pass) {
            if($level !== NULL) {
                $level= $row["level"];
            }
            return 0;
        } else {
            $reason = "Invalid username/password";
        }
    }

    if(empty($reason)) {
        $reason = $mysqli->error;
    }

    $error = "Login failed (" . $reason . ")";

    return 1;
}
