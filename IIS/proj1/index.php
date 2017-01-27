<!DOCTYPE html>

<?php
session_start();

include "db.php";
include "utils.php";

session_timeout();

$error = "";

if(isset($_POST["username"]) && isset($_POST["password"])) {
    $user = EscapeShellCmd($_POST["username"]);
    $pass = hash_pass($_POST["password"]);

    if(user_login($user, $pass, $error) === 0) {
        $_SESSION["loggedin"] = 1;
        $_SESSION["username"] = $user;
        $_SESSION["password"] = $pass;
        $_SESSION["lastact"] = time();
        header("Location: is.php");
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
        <div style="color:#FF0000">
            <?php echo $error; ?>
        </div>
        <button class="btn btn-lg btn-primary btn-block" type="submit">Log in</button>
      </form>
    </div> <!-- /container -->
  </body>
</html>
