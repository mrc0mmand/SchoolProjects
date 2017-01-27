<!DOCTYPE html>
<?php
session_start();

include "db.php";
include "utils.php";
include "actions.php";
include "adm_actions.php";

session_timeout();
$error = "";

if(!isset($_SESSION["loggedin"])) {
    header("location: index.php");
    return;
}

$user = $_SESSION["username"];
$pass = $_SESSION["password"];
$level = 0;

if(user_login($user, $pass, $error, $level) !== 0) {
    header("Location: index.php");
    exit;
}

$_SESSION["lastact"] = time();

if(!isset($_GET["method"])) {
    $_GET["method"] = "index";
}

?>

<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="author" content="">
    <link rel="icon" href="../../favicon.ico">

    <title>IIS</title>

    <!-- Bootstrap core CSS -->
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css">

    <!-- Custom styles for this template -->
    <link href="css/dashboard.css" rel="stylesheet">
    <!-- Cool tables -->
    <link rel="stylesheet" type="text/css" href="//cdn.datatables.net/1.10.12/css/jquery.dataTables.css">
    <!-- Cool selects -->
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/bootstrap-select/1.12.1/css/bootstrap-select.min.css">
    <link rel="stylesheet" type="text/css" href="css/styles.css">

    <!-- Bootstrap core JavaScript -->
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js"></script>
    <!-- Cool tables -->
    <script type="text/javascript" charset="utf8" src="//cdn.datatables.net/1.10.12/js/jquery.dataTables.js"></script>
    <!-- Cool selects -->
    <script src="https://cdnjs.cloudflare.com/ajax/libs/bootstrap-select/1.12.1/js/bootstrap-select.min.js"></script>
  </head>

  <body>

    <nav class="navbar navbar-inverse navbar-fixed-top">
      <div class="container-fluid">
        <div class="navbar-header">
          <a class="navbar-brand" href="?method=index">B-IS</a>
        </div>
        <div id="navbar" class="navbar-collapse collapse">
          <ul class="nav navbar-nav navbar-left">
          <li><a>Logged as: <?php echo $user; ?></a></li>
          </ul>
          <ul class="nav navbar-nav navbar-right">
            <li><a href="logout.php?logout">Logout</a></li>
          </ul>
        </div>
      </div>
    </nav>

    <div class="container-fluid">
      <div class="row">

        <!-- Menu - Available actions -->
        <div class="col-sm-3 col-md-2 sidebar">
            <ul class="nav nav-sidebar">
<?php
foreach($ACTIONS as $action) {
    if($level < $action[2]) {
        continue;
    }

    echo '<li><a href="?method=' . $action[0] . '">'
       . $action[1] . '</a></li>';
    echo "\n";
}

if($level > 1) {
    echo "<hr>\n";
    echo '<li><a href="?method=list-users">User control</a></li>';
}
?>
            </ul>
        </div>
        <!-- MAIN CONTENT -->
        <div class="col-sm-9 col-sm-offset-3 col-md-10 col-md-offset-2 main">
            <div style="color: #FF0000">
<?php
    if(isset($_SESSION["lasterror"])) {
        echo $_SESSION["lasterror"] . "<br><br>";
        unset($_SESSION["lasterror"]);
    }
?>
</div>
<?php

switch($_GET["method"]) {
case "index":
?>
<h1 class="page-header">B-IS</h1>

<?php
    break;
case "list-donors":
    list_donors($level);
    break;
case "list-clients":
    list_clients($level);
    break;
case "list-branches":
    list_branches($level);
    break;
case "list-invitations":
    list_invitations($level);
    break;
case "list-samples":
    list_samples($level);
    break;
case "list-collections":
    list_collections($level);
    break;
case "list-doses":
    list_doses($level);
    break;
case "list-reservations":
    list_reservations($level);
    break;
case "list-users":
    if($level < 2) {
        echo '<h1 class="page-header" style="color: #FF0000">Unauthorized</h1>';
    } else {
        list_users($level);
    }

    break;
default:

}
?>
        </div>
      </div>
    </div>

  </body>
</html>
