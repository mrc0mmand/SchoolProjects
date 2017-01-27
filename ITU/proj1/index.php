<!DOCTYPE html>

<?php
session_start();

$error = "";

if(isset($_POST["username"]) && isset($_POST["password"]) &&
    isset($_POST["server"]) && isset($_POST["database"])) {

    $user = $_POST["username"];
    $pass = $_POST["password"];
    $srv = $_POST["server"];
    $db = $_POST["database"];

    $mysqli = new mysqli($srv, $user, $pass, $db);

    if($mysqli->connect_errno) {
        $error = "Failed to connect to MySQL: (" . $mysqli->connect_errno . ") "
               . $mysqli->connect_error;
    } else {
        $_SESSION["creds"] = array($user, $pass, $srv, $db);
        header("location: dashboard.php");
    }
}
?>

<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="author" content="">
    <link rel="icon" href="favicon.ico">

    <title>ITU - Login</title>

    <!-- Bootstrap core CSS -->
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css">

    <!-- Custom styles for this template -->
    <link href="css/signin.css" rel="stylesheet">
  </head>

  <body>
    <div class="container">
      <form action="#" method="post" class="form-signin">
        <h2 class="form-signin-heading">Please log in</h2>
        <label for="inputEmail" class="sr-only">Username</label>
        <input id="inputEmail" type="text" name="username" class="form-control" placeholder="Username" required autofocus>
        <label for="inputPassword" class="sr-only">Password</label>
        <input id="inputPassword" type="password" name="password" class="form-control" placeholder="Password" required>
        <label for="inputServer" class="sr-only">Server</label>
        <input id="inputServer" type="text" name="server" class="form-control" placeholder="Server" required>
        <label for="inputDatabase" class="sr-only">Database</label>
        <input id="inputDatabase" type="text" name="database" class="form-control" placeholder="Database" required>
        <div class="checkbox">
          <label>
            <input type="checkbox" value="remember-me"> Remember me
          </label>
        </div>
        <div style="color: #cc0000">
            <?php echo $error ?>
        </div>
        <button class="btn btn-lg btn-primary btn-block" type="submit">Log in</button>
      </form>
    </div> <!-- /container -->
  </body>
</html>
