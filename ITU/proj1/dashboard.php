<!DOCTYPE html>
<?php
session_start();
$error = "";

include "actions.php";

if(!isset($_SESSION["creds"])) {
    header("location: /");
    return;
}

list($user, $pass, $srv, $db) = $_SESSION["creds"];

$mysqli = new mysqli($srv, $user, $pass, $db);

if($mysqli->connect_errno || !$mysqli->set_charset("utf8")) {
    header("location: index.php");
    //echo "Failed to connect to MySQL: (" . $mysqli->connect_errno . ") "
    //    . $mysqli->connect_error;
    exit();
}

if(isset($_POST["createTable"])) {
    $req = "CREATE TABLE " . $_POST["tableName"] . " (";
    $first = true;
    $cols = $_POST["table"];
    for($i = 0; $i < count($cols["colname"]); $i++) {
        if(!empty($cols["colname"][$i]) && !empty($cols["coltype"][$i])) {
            if($first) {
                $first = false;
            } else {
                $req .= ", ";
            }

            $req .= $cols["colname"][$i] . " " . $cols["coltype"][$i];
        }
    }

    $req .= ")";
    if(!$mysqli->query($req)) {
        $error = "Table creation failed (" . $mysqli->errno . ") " . $mysqli->error;
    } else {
        header("Location: " . $_SERVER["PHP_SELF"], true, 302);
    }
}

if(!isset($_GET["action"])) {
    $_GET["action"] = "main";
}

?>

<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="author" content="">
    <link rel="icon" href="../../favicon.ico">

    <title>ITU - DB Editor</title>

    <!-- Bootstrap core CSS -->
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css">
    <!-- Custom styles for this template -->
    <link href="css/dashboard.css" rel="stylesheet">
    <link rel="stylesheet" type="text/css" href="//cdn.datatables.net/1.10.12/css/jquery.dataTables.css">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/bootstrap-select/1.12.1/css/bootstrap-select.min.css">


    <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js"></script>
    <script type="text/javascript" charset="utf8" src="//cdn.datatables.net/1.10.12/js/jquery.dataTables.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/bootstrap-select/1.12.1/js/bootstrap-select.min.js"></script>

<script>
$(document).ready(function() {
    $("#dropTableIcon").click(function() {
        $("#dropTableName strong").text($("#listedTableName").text());
        $("#dropTable").modal("show");
    });

    $("#editTableIcon").click(function() {
        $("#editTable").modal("show");
    });

    $("#addRowTableIcon").click(function() {
        $("#insertRow").modal("show");
    });
});
</script>

  </head>

  <body>
<?php
include "modals.php";
?>
    <nav class="navbar navbar-inverse navbar-fixed-top">
      <div class="container-fluid">
        <div class="navbar-header">
          <button type="button" class="navbar-toggle collapsed" data-toggle="collapse" data-target="#navbar" aria-expanded="false" aria-controls="navbar">
            <span class="sr-only">Toggle navigation</span>
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
          </button>
          <a class="navbar-brand" href="#">
<?php echo "$db ($user@$srv)"; ?>
        </a>
        </div>
        <div id="navbar" class="navbar-collapse collapse">
          <ul class="nav navbar-nav navbar-right">
            <li><a data-toggle="modal" data-target="#createTable" href="#">Create a table</a></li>
            <li><a data-toggle="modal" data-target="#execStmt" href="#">Execute</a></li>
            <li><a data-toggle="modal" data-target="#settings" href="#">Settings</a></li>
            <li><a data-toggle="modal" data-target="#account" href="#">Account</a></li>
            <li><a href="#">Logout</a></li>
          </ul>
        </div>
      </div>
    </nav>

    <div class="container-fluid">
      <div class="row">

        <!-- Menu - Table list -->
        <div class="col-sm-3 col-md-2 sidebar">
            <ul class="nav nav-sidebar">
<?php
$res =$mysqli->query("SHOW TABLES");
while($row = mysqli_fetch_array($res)) {
    echo '<li><a href="?action=list&table=' . $row[0] . '"">' . $row[0] . "</a></li>";
}
?>
            </ul>
        </div>

        <div class="col-sm-9 col-sm-offset-3 col-md-10 col-md-offset-2 main">
          <div class="row" style="color: #cc0000">
<?php
if(isset($error) && !empty($error)) {
    echo "Error: " . $error;
}
?>
          </div>
<?php
switch($_GET["action"]) {
case "list":
    echo '<h1 class="page-header">'.
        'Table: <span id="listedTableName">' . $_GET["table"] . '</span>' .
        '&nbsp;<img id="dropTableIcon" src="assets/close.png" width="16px">' .
        '&nbsp;<img id="editTableIcon" src="assets/edit.png" width="16px">' .
        '&nbsp;<img id="addRowTableIcon" src="assets/plus.png" width="16px">' .
        '</h1>';
    list_table($_GET["table"]);
    break;
default:
?>
    <h1 class="page-header">Overview</h1>
<?php
}

?>

          </div>
        </div>
      </div>
    </div>
  </body>
</html>
