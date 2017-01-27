<?php

function level_to_string($level) {
    $levelstr = "UKNOWN";

    switch($level) {
    case 0:
        $levelstr = "GUEST";
        break;
    case 1:
        $levelstr = "USER";
        break;
    case 2:
        $levelstr = "ADMIN";
        break;
    }

    return $levelstr;
}

function list_users($level) {
    if($level < 2)
        return;
    global $mysqli;
    $id = (isset($_GET["id"]) ? $_GET["id"] : NULL);

    if($id !== NULL) {
        $stmt = $mysqli->prepare("
            SELECT * FROM users WHERE id = ?;");
        $stmt->bind_param("i", $id);
        $stmt->execute();
        $res = $stmt->get_result();
    } else {
        $res = $mysqli->query("SELECT * FROM users");
    }

    if(!$res) {
        echo "Error: $mysqli->error";
        return;
    }
?>
<script>
$(document).ready(function() {
    $("#user-list").DataTable();

    $("#newUserForm").submit(function(event) {
        var action = "?method=list-users&id=";
        $.ajax({
            type: "POST",
            url: "submit.php",
            data: $("#newUserForm").serialize(),
            success: function(data) {
                if(data) {
                    var res = jQuery.parseJSON(data);
                    if(res.error) {
                        $("#newUserError").html(res.error);
                    } else {
                        window.location.replace(action + res.id);
                    }
                }
            }
        });

        event.preventDefault();
    });

    $("#user-list tr").click(function() {
        var columns = $(this).children().map(function() {
            return $(this).text();
        }).toArray();

        $("#insertType").val("user-update");
        $("#newUserForm").append('<input type="hidden" name="userID" id="userID" value="' + parseInt(columns[0]) + '">');
        $("#userName").val(columns[1]);
        $("#userEmail").val(columns[2]);
        $("#userLevel option").filter(function() {
            return $(this).text() == columns[3];
        }).prop("selected", true);

        $("#userPass1").removeProp("required").parent().parent().removeClass("required");
        $("#userPass2").removeProp("required").parent().parent().removeClass("required");
        $("#newUser").modal("show");
    });

    $("#newUser").on("hidden.bs.modal", function() {
        $("#userID").remove();
        $("#insertType").val("user");
        $("#userName").val("");
        $("#userEmail").val("");
        $("#userPass1").val("");
        $("#userPass2").val("");
        $("#userLevel option").first().prop("selected", true);
        $("#userPass1").prop("required", true).parent().parent().addClass("required");
        $("#userPass2").prop("required", true).parent().parent().addClass("required");
        $("#newUserError").text("");
    });
});
</script>
<button style="float: right" type="button" class="btn btn-primary btn-lg" data-toggle="modal" data-target="#newUser">New user</button>
<div class="modal fade" id="newUser" tabindex="-1" data-backdrop="static" role="dialog" aria-labelledby="newUserLabel">
  <div class="modal-dialog" role="document">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
        <h4 class="modal-title" id="newUserLabel">Add a new user</h4>
      </div>
      <div class="modal-body">
        <form id="newUserForm" class="form-horizontal">
          <input type="hidden" id="insertType" name="insertType" value="user">
          <div class="form-group required">
            <label for="userName" class="col-sm-2 control-label">Name</label>
            <div class="col-sm-10">
              <input type="text" class="form-control" name="userName" id="userName" placeholder="Name" required>
            </div>
          </div>
          <div class="form-group required">
            <label for="userEmail" class="col-sm-2 control-label">Email</label>
            <div class="col-sm-10">
              <input type="email" class="form-control" name="userEmail" id="userEmail" placeholder="Email" required>
            </div>
          </div>
          <div class="form-group required">
            <label for="userLevel" class="col-sm-2 control-label">Level</label>
            <div class="col-sm-10">
            <select class="form-control" name="userLevel" id="userLevel">
                <option value="0">GUEST</option>
                <option value="1">USER</option>
                <option value="2">ADMIN</option>
            </select>
            </div>
          </div>
          <div class="form-group required">
            <label for="userPass1" class="col-sm-2 control-label">Password</label>
            <div class="col-sm-10">
              <input type="password" class="form-control" name="userPass1" id="userPass1" placeholder="Password" required>
            </div>
          </div>
          <div class="form-group required">
            <label for="userPass2" class="col-sm-2 control-label">Repeat password</label>
            <div class="col-sm-10">
              <input type="password" class="form-control" name="userPass2" id="userPass2" placeholder="Password" required>
            </div>
          </div>
        </form>
        <div id="newUserError" style="color: #FF0000">
        </div>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
        <button id="newUserSubmit" form="newUserForm" type="submit" class="btn btn-primary">Save changes</button>
      </div>
    </div>
  </div>
</div>
<h1 class="page-header">Users</h1>
<div class="table-responsive">
    <table id="user-list" class="table table-hover table-striped">
        <thead>
        <tr>
            <th>#</th>
            <th>Username</th>
            <th>Email</th>
            <th>Level</th>
            <th>Remove</th>
        </tr>
        </thead>
        <tbody>
<?php
    while($row = $res->fetch_assoc()) {
        echo "<tr>";
        echo "<td>" . $row["id"] . "</td>";
        echo "<td>" . $row["username"] . "</td>";
        echo "<td>" . $row["email"] . "</td>";
        echo "<td>" . level_to_string($row["level"]) . "</td>";
        echo "<td><a href='adm_submit.php?dropType=user&userID=" . $row["id"]
            . "'><img src='assets/close.png' width='16px'></a></td>";
        echo "</tr>";
    }
?>
    </tbody>
    </table>
</div>
<?php
}

?>
