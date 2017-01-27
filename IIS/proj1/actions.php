<?php

// action - name - min_level
$ACTIONS = array(
    array("list-donors",       "Donors",         1),
    array("list-clients",      "Clients",        1),
    array("list-branches",     "Branches",       1),
    array("list-invitations",  "Invitations",    1),
    array("list-samples",      "Samples",        1),
    array("list-collections",  "Collections",    1),
    array("list-doses",        "Doses",          0),
    array("list-reservations", "Reservations",   1),
);

$INSURANCE = array(
    "VZP",
    "VoZP",
    "CPZP",
);

usort($ACTIONS, function($a, $b) {
        return strcmp($a[1], $b[1]);
});

function list_donors($level) {
    if($level < 1)
        return;
    global $mysqli;
    global $INSURANCE;
    $id = (isset($_GET["id"]) ? $_GET["id"] : NULL);

    if($id !== NULL) {
        $stmt = $mysqli->prepare("
            SELECT d.id, d.name, d.surname, d.sex, d.weight, d.birth_date,
            d.personal_id, d.address, d.insurance, d.blood_type,
            d.reg_date, d.last_collection,
            CONCAT(b.name, ' - ', b.address) AS br,
            b.id AS b_id
            FROM donor AS d, branch AS b
            WHERE d.branch_id = b.id
            AND d.id = ?;");
        $stmt->bind_param("i", $id);
        $stmt->execute();
        $res = $stmt->get_result();
    } else {
        $res = $mysqli->query("
            SELECT d.id, d.name, d.surname, d.sex, d.weight, d.birth_date,
            d.personal_id, d.address, d.insurance, d.blood_type,
            d.reg_date, d.last_collection,
            CONCAT(b.name, ' - ', b.address) AS br,
            b.id AS b_id
            FROM donor AS d, branch AS b
            WHERE d.branch_id = b.id;");
    }

    if(!$res) {
        echo "Error: $mysqli->error";
        return;
    }
?>
<script>
$(document).ready(function() {
    $("#donor-list").DataTable();

    $("#newDonorForm").submit(function(event) {
        var action = "?method=list-donors&id=";
        $.ajax({
            type: "POST",
            url: "submit.php",
            data: $("#newDonorForm").serialize(),
            success: function(data) {
                if(data) {
                    var res = jQuery.parseJSON(data);
                    if(res.error) {
                        $("#newDonorError").html(res.error);
                    } else {
                        window.location.replace(action + res.id);
                    }
                }
            }
        });

        event.preventDefault();
    });
});
</script>
<button style="float: right" type="button" class="btn btn-primary btn-lg" data-toggle="modal" data-target="#newDonor">New donor</button>
<div class="modal fade" id="newDonor" tabindex="-1" data-backdrop="static" role="dialog" aria-labelledby="newDonorLabel">
  <div class="modal-dialog" role="document">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
        <h4 class="modal-title" id="newDonorLabel">Add a new donor</h4>
      </div>
      <div class="modal-body">
        <form id="newDonorForm" class="form-horizontal">
          <input type="hidden" name="insertType" value="donor">
          <div class="form-group required">
            <label for="donorName" class="col-sm-2 control-label">Name</label>
            <div class="col-sm-10">
              <input type="text" class="form-control" name="donorName" id="donorName" placeholder="Name" required>
            </div>
          </div>
          <div class="form-group required">
            <label for="donorSurname" class="col-sm-2 control-label">Surname</label>
            <div class="col-sm-10">
              <input type="text" class="form-control" name="donorSurname" id="donorSurname" placeholder="Surname" required>
            </div>
          </div>
          <div class="form-group required">
            <label for="donorSex" class="col-sm-2 control-label">Sex</label>
            <div class="col-sm-10">
            <select class="form-control selectpicker" name="donorSex" id="donorSex">
                <option>M</option>
                <option>F</option>
            </select>
            </div>
          </div>
          <div class="form-group required">
            <label for="donorWeight" class="col-sm-2 control-label">Weight</label>
            <div class="col-sm-10">
              <input type="number" min="0" class="form-control" name="donorWeight" id="donorWeight" placeholder="Weight" required>
            </div>
          </div>
          <div class="form-group required">
            <label for="donorBirth" class="col-sm-2 control-label">Birth date</label>
            <div class="col-sm-10">
              <input type="date" class="form-control" name="donorBirth" id="donorBirth" placeholder="YYYY-MM-DD" required>
            </div>
          </div>
          <div class="form-group required">
            <label for="donorPersonalID" class="col-sm-2 control-label">Personal ID</label>
            <div class="col-sm-10">
              <input type="text" class="form-control" name="donorPersonalID" id="donorPersonalID" placeholder="Personal ID" required>
            </div>
          </div>
          <div class="form-group required">
            <label for="donorAddress" class="col-sm-2 control-label">Address</label>
            <div class="col-sm-10">
              <input type="text" class="form-control" name="donorAddress" id="donorAddress" placeholder="Address" required>
            </div>
          </div>
          <div class="form-group required">
            <label for="donorInsurance" class="col-sm-2 control-label">Insurance</label>
            <div class="col-sm-10">
                <select class="form-control selectpicker" name="donorInsurance" id="donorInsurance">
                    <?php
                        foreach($INSURANCE as $item) {
                            echo "<option>" . $item . "</option>";
                        }
                    ?>
                </select>
            </div>
          </div>
          <div class="form-group required">
            <label for="donorBlood" class="col-sm-2 control-label">Blood type</label>
            <div class="col-sm-10">
            <select class="form-control" name="donorBlood" id="donorBlood">
                <option>0+</option>
                <option>0-</option>
                <option>A+</option>
                <option>A-</option>
                <option>B+</option>
                <option>B-</option>
                <option>AB+</option>
                <option>AB-</option>
            </select>
            </div>
          </div>
          <div class="form-group required">
            <label for="donorBranch" class="col-sm-2 control-label">Branch</label>
            <div class="col-sm-10">
            <select class="form-control selectpicker" data-live-search="true" name="donorBranch" id="donorBranch">
<?php
    $res2 = $mysqli->query("SELECT * FROM branch");
    while($row = $res2->fetch_assoc()) {
        echo "<option value='" . $row["id"] . "'>" . $row["name"] . " - "
            . $row["address"] . "</option>\n";
    }
?>
            </select>
            </div>
          </div>
        </form>
        <div id="newDonorError" style="color: #FF0000">
        </div>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
        <button id="newDonorSubmit" form="newDonorForm" type="submit" class="btn btn-primary">Save changes</button>
      </div>
    </div>
  </div>
</div>
<h1 class="page-header">Donors</h1>
<div class="table-responsive">
    <table id="donor-list" class="table table-hover table-striped">
        <thead>
        <tr>
            <th>#</th>
            <th>Name</th>
            <th>Surname</th>
            <th>Sex</th>
            <th>Weight</th>
            <th>Birth date</th>
            <th>Personal ID</th>
            <th>Address</th>
            <th>Insurance</th>
            <th>BT</th>
            <th>Reg. date</th>
            <th>Last collection</th>
            <th>Branch</th>
            <th>&nbsp;</th>
        </tr>
        </thead>
        <tbody>
<?php
    while($row = $res->fetch_assoc()) {
        echo "<tr>";
        echo "<td>" . $row["id"] . "</td>";
        echo "<td>" . $row["name"] . "</td>";
        echo "<td>" . $row["surname"] . "</td>";
        echo "<td>" . $row["sex"] . "</td>";
        echo "<td>" . $row["weight"] . "</td>";
        echo "<td>" . $row["birth_date"] . "</td>";
        echo "<td>" . $row["personal_id"] . "</td>";
        echo "<td>" . $row["address"] . "</td>";
        echo "<td>" . $row["insurance"] . "</td>";
        echo "<td>" . $row["blood_type"] . "</td>";
        echo "<td>" . $row["reg_date"] . "</td>";
        echo "<td>" . $row["last_collection"] . "</td>";
        echo "<td><a href='?method=list-branches&id=" . $row["b_id"] . "'>"
           . $row["br"] . "</a></td>";
        echo "<td><a href='?method=list-collections&donor=" . $row["id"]
            . "'>Collections</a></td>";
        echo "</tr>";
    }
?>
    </tbody>
    </table>
</div>
<?php
}

function list_clients($level) {
    if($level < 1)
        return;
    global $mysqli;
    $id = (isset($_GET["id"]) ? $_GET["id"] : NULL);

    if($id !== NULL) {
        $stmt = $mysqli->prepare("
            SELECT c.id, c.type, c.name, c.surname, c.address,
            CONCAT(b.name, ' - ', b.address) AS br,
            b.id AS b_id
            FROM client AS c, branch as b
            WHERE c.branch_id = b.id
            AND c.id = ?;");
        $stmt->bind_param("i", $id);
        $stmt->execute();
        $res = $stmt->get_result();
    } else {
        $res = $mysqli->query("
            SELECT c.id, c.type, c.name, c.surname, c.address,
            CONCAT(b.name, ' - ', b.address) AS br,
            b.id AS b_id
            FROM client AS c, branch as b
            WHERE c.branch_id = b.id;");
    }

    if(!$res) {
        echo "Error: $mysqli->error";
        return;
    }
?>
<script>
$(document).ready(function() {
    $("#client-list").DataTable();

    $("#newClientForm").submit(function(event) {
        var action = "?method=list-clients&id=";
        $.ajax({
            type: "POST",
            url: "submit.php",
            data: $("#newClientForm").serialize(),
            success: function(data) {
                if(data) {
                    var res = jQuery.parseJSON(data);
                    if(res.error) {
                        $("#newClientError").html(res.error);
                    } else {
                        window.location.replace(action + res.id);
                    }
                }
            }
        });

        event.preventDefault();
    });
});
</script>
<button style="float: right" type="button" class="btn btn-primary btn-lg" data-toggle="modal" data-target="#newClient">New client</button>
<div class="modal fade" id="newClient" tabindex="-1" data-backdrop="static" role="dialog" aria-labelledby="newClientLabel">
  <div class="modal-dialog" role="document">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
        <h4 class="modal-title" id="newClientLabel">Add a new client</h4>
      </div>
      <div class="modal-body">
        <form id="newClientForm" class="form-horizontal">
          <input type="hidden" name="insertType" value="client">
          <div class="form-group required">
            <label for="clientType" class="col-sm-2 control-label">Type</label>
            <div class="col-sm-10">
            <select class="form-control" name="clientType" id="clientType">
                <option>PERSON</option>
                <option>COMPANY</option>
            </select>
            </div>
          </div>
          <div class="form-group required">
            <label for="clientName" class="col-sm-2 control-label">Name</label>
            <div class="col-sm-10">
              <input type="text" class="form-control" name="clientName" id="clientName" placeholder="Name" required>
            </div>
          </div>
          <div class="form-group">
            <label for="clientSurname" class="col-sm-2 control-label">Surname</label>
            <div class="col-sm-10">
              <input type="text" class="form-control" name="clientSurname" id="clientSurname" placeholder="Surname">
            </div>
          </div>
          <div class="form-group required">
            <label for="clientAddress" class="col-sm-2 control-label">Address</label>
            <div class="col-sm-10">
              <input type="text" class="form-control" name="clientAddress" id="clientAddress" placeholder="Address" required>
            </div>
          </div>
          <div class="form-group required">
            <label for="clientBranch" class="col-sm-2 control-label">Branch</label>
            <div class="col-sm-10">
            <select class="form-control" name="clientBranch" id="clientBranch">
<?php
    $res2 = $mysqli->query("SELECT * FROM branch");
    while($row = $res2->fetch_assoc()) {
        echo "<option value='" . $row["id"] . "'>" . $row["name"] . " - "
            . $row["address"] . "</option>\n";
    }
?>
            </select>
            </div>
          </div>
        </form>
        <div id="newClientError" style="color: #FF0000">
        </div>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
        <button id="newClientSubmit" form="newClientForm" type="submit" class="btn btn-primary">Save changes</button>
      </div>
    </div>
  </div>
</div>
<h1 class="page-header">Clients</h1>
<div class="table-responsive">
    <table id="client-list" class="table table-hover table-striped">
        <thead>
        <tr>
            <th>#</th>
            <th>Type</th>
            <th>Name</th>
            <th>Surname</th>
            <th>Address</th>
            <th>Branch</th>
        </tr>
        </thead>
        <tbody>
<?php
    while($row = $res->fetch_assoc()) {
        echo "<tr>";
        echo "<td>" . $row["id"] . "</td>";
        echo "<td>" . $row["type"] . "</td>";
        echo "<td>" . $row["name"] . "</td>";
        echo "<td>" . $row["surname"] . "</td>";
        echo "<td>" . $row["address"] . "</td>";
        echo "<td><a href='?method=list-branches&id=" . $row["b_id"] . "'>"
           . $row["br"] . "</a></td>";
        echo "</tr>";
    }
?>
    </tbody>
    </table>
</div>
<?php
}

function list_branches($level) {
    if($level < 1)
        return;
    global $mysqli;
    $id = (isset($_GET["id"]) ? $_GET["id"] : NULL);

    if($id !== NULL) {
        $stmt = $mysqli->prepare(
            "SELECT id, name, address FROM branch WHERE id = ?;");
        $stmt->bind_param("i", $id);
        $stmt->execute();
        $res = $stmt->get_result();
    } else {
        $res = $mysqli->query("
            SELECT id, name, address FROM branch;");
    }

    if(!$res) {
        echo "Error: $mysqli->error";
        return;
    }
?>
<script>
$(document).ready(function() {
    $("#branch-list").DataTable();

    $("#newBranchForm").submit(function(event) {
        var action = "?method=list-branches&id=";
        $.ajax({
            type: "POST",
            url: "submit.php",
            data: $("#newBranchForm").serialize(),
            success: function(data) {
                if(data) {
                    var res = jQuery.parseJSON(data);
                    if(res.error) {
                        $("#newBranchError").html(res.error);
                    } else {
                        window.location.replace(action + res.id);
                    }
                }
            }
        });

        event.preventDefault();
    });
});

</script>
<button style="float: right" type="button" class="btn btn-primary btn-lg" data-toggle="modal" data-target="#newBranch">New branch</button>
<div class="modal fade" id="newBranch" tabindex="-1" data-backdrop="static" role="dialog" aria-labelledby="newBranchLabel">
  <div class="modal-dialog" role="document">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
        <h4 class="modal-title" id="newBranchLabel">Add a new branch</h4>
      </div>
      <div class="modal-body">
        <form id="newBranchForm" class="form-horizontal">
          <input type="hidden" name="insertType" value="branch">
          <div class="form-group required">
            <label for="branchName" class="col-sm-2 control-label">Name</label>
            <div class="col-sm-10">
              <input type="text" class="form-control" name="branchName" id="branchName" placeholder="Branch name" required>
            </div>
          </div>
          <div class="form-group required">
            <label for="branchAddress" class="col-sm-2 control-label">Address</label>
            <div class="col-sm-10">
              <input type="text" class="form-control" name="branchAddress" id="branchAddress" placeholder="Branch address" required>
            </div>
          </div>
        </form>
        <div id="newBranchError" style="color: #FF0000">
        </div>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
        <button id="newBranchSubmit" form="newBranchForm" type="submit" class="btn btn-primary">Save changes</button>
      </div>
    </div>
  </div>
</div>
<h1 class="page-header">Branches</h1>
<div class="table-responsive">
    <table id="branch-list" class="table table-hover table-striped">
        <thead>
        <tr>
            <th>#</th>
            <th>Name</th>
            <th>Address</th>
        </tr>
        </thead>
        <tbody>
<?php
    while($row = $res->fetch_assoc()) {
        echo "<tr>";
        echo "<td>" . $row["id"] . "</td>";
        echo "<td>" . $row["name"] . "</td>";
        echo "<td>" . $row["address"] . "</td>";
        echo "</tr>";
    }
?>
    </tbody>
    </table>
</div>
<?php
}

function list_invitations($level) {
    if($level < 1)
        return;
    global $mysqli;
    $id = (isset($_GET["id"]) ? $_GET["id"] : NULL);

    if($id !== NULL) {
        $stmt = $mysqli->prepare("
            SELECT i.id, i.inv_date, i.content, i.state,
            CONCAT(d.name, ' ', d.surname, ' (', d.personal_id, ')') AS td,
            CONCAT(b.name, ' - ', b.address) AS ob,
            d.id AS d_id,
            b.id AS b_id
            FROM invitation AS i, donor AS d, branch as b
            WHERE i.donor_id = d.id
            AND i.branch_id = b.id
            AND i.id = ?");
        $stmt->bind_param("i", $id);
        $stmt->execute();
        $res = $stmt->get_result();
    } else {
        $res = $mysqli->query("
            SELECT i.id, i.inv_date, i.content, i.state,
            CONCAT(d.name, ' ', d.surname, ' (', d.personal_id, ')') AS td,
            CONCAT(b.name, ' - ', b.address) AS ob,
            d.id AS d_id,
            b.id AS b_id
            FROM invitation AS i, donor AS d, branch as b
            WHERE i.donor_id = d.id
            AND i.branch_id = b.id");
    }

    if(!$res) {
        echo "Error: $mysqli->error";
        return;
    }
?>
<script>
$(document).ready(function() {
    $("#invitation-list").DataTable();

    $("#newInvitationForm").submit(function(event) {
        var action = "?method=list-invitations&id=";
        $.ajax({
            type: "POST",
            url: "submit.php",
            data: $("#newInvitationForm").serialize(),
            success: function(data) {
                if(data) {
                    var res = jQuery.parseJSON(data);
                    if(res.error) {
                        $("#newInvitationError").html(res.error);
                    } else {
                        window.location.replace(action + res.id);
                    }
                }
            }
        });

        event.preventDefault();
    });
});
</script>
<button style="float: right" type="button" class="btn btn-primary btn-lg" data-toggle="modal" data-target="#newInvitation">New invitation</button>
<div class="modal fade" id="newInvitation" tabindex="-1" data-backdrop="static" role="dialog" aria-labelledby="newInvitationLabel">
  <div class="modal-dialog" role="document">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
        <h4 class="modal-title" id="newInvitationLabel">Create a new invitation</h4>
      </div>
      <div class="modal-body">
        <form id="newInvitationForm" class="form-horizontal">
          <input type="hidden" name="insertType" value="invitation">
          <div class="form-group required">
            <label for="invitationName" class="col-sm-2 control-label">Donor</label>
            <div class="col-sm-10">
                <select class="form-control selectpicker" data-live-search="true" name="invitationName" id="invitationName">
<?php
    $donors = $mysqli->query("
        SELECT d.name, d.surname, d.personal_id,
            b.id AS b_id,
            d.id AS d_id
        FROM donor AS d, branch AS b
        WHERE d.branch_id = b.id
    ");
    while($row = $donors->fetch_assoc()) {
        echo "<option value='" . $row["d_id"] . ":" . $row["b_id"] . "'>"
            . $row["name"] . " " . $row["surname"] . " ("
            . $row["personal_id"] . ")</option>\n";
    }
?>
                </select>
            </div>
          </div>
          <div class="form-group required">
            <label for="invitationText" class="col-sm-2 control-label">Text</label>
            <div class="col-sm-10">
              <textarea class="form-control" name="invitationText" id="invitationText" placeholder="Invitation text" required></textarea>
            </div>
          </div>
        </form>
        <div id="newInvitationError" style="color: #FF0000">
        </div>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
        <button id="newInvitationSubmit" form="newInvitationForm" type="submit" class="btn btn-primary">Save changes</button>
      </div>
    </div>
  </div>
</div>
<h1 class="page-header">Invitations</h1>
<div class="table-responsive">
    <table id="invitation-list" class="table table-hover table-striped">
        <thead>
        <tr>
            <th>#</th>
            <th>Date</th>
            <th>Content</th>
            <th>State</th>
            <th>Donor</th>
            <th>Branch</th>
        </tr>
        </thead>
        <tbody>
<?php
    while($row = $res->fetch_assoc()) {
        echo "<tr>";
        echo "<td>" . $row["id"] . "</td>";
        echo "<td>" . $row["inv_date"] . "</td>";
        echo "<td>" . $row["content"] . "</td>";
        echo "<td>" . $row["state"] . "</td>";
        echo "<td><a href='?method=list-donors&id=" . $row["d_id"] . "'>"
           . $row["td"] . "</a></td>";
        echo "<td><a href='?method=list-branches&id=" . $row["b_id"] . "'>"
           . $row["ob"] . "</a></td>";
        echo "</tr>";
    }
?>
    </tbody>
    </table>
</div>
<?php
}

function list_samples($level) {
    if($level < 1)
        return;
    global $mysqli;
    $id = (isset($_GET["id"]) ? $_GET["id"] : NULL);

    if($id !== NULL) {
        $stmt = $mysqli->prepare("
            SELECT s.id, s.cbc, s.suitable, s.reason, s.sample_date,
            CONCAT(d.name, ' ', d.surname, ' (', d.personal_id, ')') AS td,
            d.id AS d_id
            FROM sample AS s, donor AS d
            WHERE s.donor_id = d.id
            AND s.id = ?");
        $stmt->bind_param("i", $id);
        $stmt->execute();
        $res = $stmt->get_result();
    } else {
        $res = $mysqli->query("
            SELECT s.id, s.cbc, s.suitable, s.reason, s.sample_date,
            CONCAT(d.name, ' ', d.surname, ' (', d.personal_id, ')') AS td,
            d.id AS d_id
            FROM sample AS s, donor AS d
            WHERE s.donor_id = d.id");
    }

    if(!$res) {
        echo "Error: $mysqli->error";
        return;
    }
?>
<script>
$(document).ready(function() {
    $("#sample-list").DataTable();

    $("#newSampleForm").submit(function(event) {
        var action = "?method=list-samples&id=";
        $.ajax({
            type: "POST",
            url: "submit.php",
            data: $("#newSampleForm").serialize(),
            success: function(data) {
                if(data) {
                    var res = jQuery.parseJSON(data);
                    if(res.error) {
                        $("#newSampleError").html(res.error);
                    } else {
                        window.location.replace(action + res.id);
                    }
                }
            }
        });

        event.preventDefault();
    });
});
</script>
<button style="float: right" type="button" class="btn btn-primary btn-lg" data-toggle="modal" data-target="#newSample">New sample</button>
<div class="modal fade" id="newSample" tabindex="-1" data-backdrop="static" role="dialog" aria-labelledby="newSampleLabel">
  <div class="modal-dialog" role="document">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
        <h4 class="modal-title" id="newSampleLabel">Add a new sample</h4>
      </div>
      <div class="modal-body">
        <form id="newSampleForm" class="form-horizontal">
          <input type="hidden" name="insertType" value="sample">
          <div class="form-group required">
            <label for="sampleName" class="col-sm-2 control-label">Donor</label>
            <div class="col-sm-10">
                <select class="form-control selectpicker" data-live-search="true" name="sampleName" id="sampleName">
<?php
    $donors = $mysqli->query("
        SELECT d.name, d.surname, d.personal_id,
            b.id AS b_id,
            d.id AS d_id
        FROM donor AS d, branch AS b
        WHERE d.branch_id = b.id
    ");
    while($row = $donors->fetch_assoc()) {
        echo "<option value='" . $row["d_id"] . ":" . $row["b_id"] . "'>"
            . $row["name"] . " " . $row["surname"] . " ("
            . $row["personal_id"] . ")</option>\n";
    }
?>
                </select>
            </div>
          </div>
          <div class="form-group required">
            <label for="sampleCBC" class="col-sm-2 control-label">CBC</label>
            <div class="col-sm-10">
              <textarea class="form-control" name="sampleCBC" id="sampleCBC" placeholder="CBC" required></textarea>
            </div>
          </div>
          <div class="form-group required">
            <label for="sampleSuitable" class="col-sm-2 control-label">Suitable</label>
            <div class="col-sm-10">
                <input type="checkbox" id="sampleSuitable" name="sampleSuitable">
            </div>
          </div>
          <div class="form-group">
            <label for="sampleReason" class="col-sm-2 control-label">Reason</label>
            <div class="col-sm-10">
              <textarea class="form-control" name="sampleReason" id="sampleReason" placeholder="Reason if unsuitable"></textarea>
            </div>
          </div>
        </form>
        <div id="newSampleError" style="color: #FF0000">
        </div>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
        <button id="newSampleSubmit" form="newSampleForm" type="submit" class="btn btn-primary">Save changes</button>
      </div>
    </div>
  </div>
</div>
<h1 class="page-header">Samples</h1>
<div class="table-responsive">
    <table id="sample-list" class="table table-hover table-striped">
        <thead>
        <tr>
            <th>#</th>
            <th>CBC</th>
            <th>Suitable</th>
            <th>Reason</th>
            <th>Date</th>
            <th>Donor</th>
        </tr>
        </thead>
        <tbody>
<?php
    while($row = $res->fetch_assoc()) {
        echo "<tr>";
        echo "<td>" . $row["id"] . "</td>";
        echo "<td>" . $row["cbc"] . "</td>";
        echo "<td>" . ($row["suitable"] ? "Yes" : "No") . "</td>";
        echo "<td>" . $row["reason"] . "</td>";
        echo "<td>" . $row["sample_date"] . "</td>";
        echo "<td><a href='?method=list-donors&id=" . $row["d_id"] . "'>"
           . $row["td"] . "</a></td>";
        echo "</tr>";
    }
?>
    </tbody>
    </table>
</div>
<?php
}

function list_collections($level) {
    if($level < 1)
        return;
    global $mysqli;
    $id = (isset($_GET["id"]) ? $_GET["id"] : NULL);
    $donor = (isset($_GET["donor"]) ? $_GET["donor"] : NULL);

    if($id !== NULL) {
        $stmt = $mysqli->prepare("
            SELECT c.id, c.collection_date, c.type, c.complications,
            c.collection_done,
            CONCAT(d.name, ' ', d.surname, ' (', d.personal_id, ')') AS td,
            d.id AS d_id,
            s.id AS s_id
            FROM collection AS c, sample AS s, donor AS d
            WHERE c.donor_id = d.id
            AND c.sample_id = s.id
            AND c.id = ?");
        $stmt->bind_param("i", $id);
        $stmt->execute();
        $res = $stmt->get_result();
    } else if($donor != NULL) {
        $stmt = $mysqli->prepare("
            SELECT c.id, c.collection_date, c.type, c.complications,
            c.collection_done,
            CONCAT(d.name, ' ', d.surname, ' (', d.personal_id, ')') AS td,
            d.id AS d_id,
            s.id AS s_id
            FROM collection AS c, sample AS s, donor AS d
            WHERE c.donor_id = d.id
            AND c.sample_id = s.id
            AND c.donor_id = ?");
        $stmt->bind_param("i", $donor);
        $stmt->execute();
        $res = $stmt->get_result();
    } else {
        $res = $mysqli->query("
            SELECT c.id, c.collection_date, c.type, c.complications,
            c.collection_done,
            CONCAT(d.name, ' ', d.surname, ' (', d.personal_id, ')') AS td,
            d.id AS d_id,
            s.id AS s_id
            FROM collection AS c, sample AS s, donor AS d
            WHERE c.donor_id = d.id
            AND c.sample_id = s.id");
    }

    if(!$res) {
        echo "Error: $mysqli->error";
        return;
    }
?>
<script>
$(document).ready(function() {
    $("#collection-list").DataTable();

    $("#newCollectionForm").submit(function(event) {
        var action = "?method=list-collections&id=";
        $.ajax({
            type: "POST",
            url: "submit.php",
            data: $("#newCollectionForm").serialize(),
            success: function(data) {
                if(data) {
                    var res = jQuery.parseJSON(data);
                    if(res.error) {
                        $("#newCollectionError").html(res.error);
                    } else {
                        window.location.replace(action + res.id);
                    }
                }
            }
        });

        event.preventDefault();
    });
});
</script>
<button style="float: right" type="button" class="btn btn-primary btn-lg" data-toggle="modal" data-target="#newCollection">New collection</button>
<div class="modal fade" id="newCollection" tabindex="-1" data-backdrop="static" role="dialog" aria-labelledby="newCollectionLabel">
  <div class="modal-dialog" role="document">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
        <h4 class="modal-title" id="newCollectionLabel">Add a new collection</h4>
      </div>
      <div class="modal-body">
        <form id="newCollectionForm" class="form-horizontal">
          <input type="hidden" name="insertType" value="collection">
          <div class="form-group required">
            <label for="collectionSample" class="col-sm-2 control-label">Sample</label>
            <div class="col-sm-10">
                <select class="form-control selectpicker" data-live-search="true" name="collectionSample" id="collectionSample">
<?php
    $samples = $mysqli->query("
        SELECT CONCAT(d.name, ' ', d.surname) AS dn, d.personal_id,
               s.sample_date,
               s.id AS s_id,
               d.id AS d_id
        FROM donor AS d, sample AS s
        WHERE s.donor_id = d.id
    ");
    while($row = $samples->fetch_assoc()) {
        echo "<option value='" . $row["d_id"] . ":" . $row["s_id"] . "'>"
            . $row["dn"] . " (" . $row["personal_id"] . ") - "
            . $row["sample_date"] . "</option>\n";
    }
?>
                </select>
            </div>
          </div>
          <div class="form-group required">
            <label for="collectionType" class="col-sm-2 control-label">Type</label>
            <div class="col-sm-10">
                <select class="form-control selectpicker" name="collectionType" id="collectionType">
                    <option value="BLOOD">Blood</option>
                    <option value="PLASMA">Plasma</option>
                    <option value="PLATELETS">Platelets</option>

                </select>
            </div>
          </div>
          <div class="form-group required">
            <label for="collectionSuccessful" class="col-sm-2 control-label">Successful</label>
            <div class="col-sm-10">
                <input type="checkbox" id="collectionSuccessful" name="collectionSuccessful">
            </div>
          </div>
          <div class="form-group">
            <label for="collectionComplications" class="col-sm-2 control-label">Complications</label>
            <div class="col-sm-10">
              <textarea class="form-control" name="collectionComplications" id="collectionComplications" placeholder="Complications description"></textarea>
            </div>
          </div>
        </form>
        <div id="newCollectionError" style="color: #FF0000">
        </div>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
        <button id="newCollectionSubmit" form="newCollectionForm" type="submit" class="btn btn-primary">Save changes</button>
      </div>
    </div>
  </div>
</div>
<h1 class="page-header">Collections</h1>
<div class="table-responsive">
    <table id="collection-list" class="table table-hover table-striped">
        <thead>
        <tr>
            <th>#</th>
            <th>Date</th>
            <th>Type</th>
            <th>Complications</th>
            <th>Successful</th>
            <th>Donor</th>
            <th>Sample</th>
        </tr>
        </thead>
        <tbody>
<?php
    while($row = $res->fetch_assoc()) {
        echo "<tr>";
        echo "<td>" . $row["id"] . "</td>";
        echo "<td>" . $row["collection_date"] . "</td>";
        echo "<td>" . $row["type"] . "</td>";
        echo "<td>" . $row["complications"] . "</td>";
        echo "<td>" . ($row["collection_done"] ? "Yes" : "No") . "</td>";
        echo "<td><a href='?method=list-donors&id=" . $row["d_id"] . "'>"
           . $row["td"] . "</a></td>";
        echo "<td><a href='?method=list-samples&id=" . $row["s_id"] . "'>#"
           . $row["s_id"] . "</a></td>";
        echo "</tr>";
    }
?>
    </tbody>
    </table>
</div>
<?php
}

function list_doses($level) {
    global $mysqli;
    $id = (isset($_GET["id"]) ? $_GET["id"] : NULL);

    if($id !== NULL) {
        $stmt = $mysqli->prepare("
            SELECT ds.id, ds.dose_date, ds.self_exclusion, ds.suitable,
            CONCAT(do.name, ' ', do.surname, ' (', do.personal_id, ')') AS td,
            do.blood_type,
            do.id AS do_id,
            c.id AS c_id
            FROM dose AS ds, collection AS c, donor AS do
            WHERE ds.collection_id = c.id
            AND ds.donor_id = do.id
            AND ds.id = ?");
        $stmt->bind_param("i", $id);
        $stmt->execute();
        $res = $stmt->get_result();
    } else {
        $res = $mysqli->query("
            SELECT ds.id, ds.dose_date, ds.self_exclusion, ds.suitable,
            CONCAT(do.name, ' ', do.surname, ' (', do.personal_id, ')') AS td,
            do.blood_type,
            do.id AS do_id,
            c.id AS c_id
            FROM dose AS ds, collection AS c, donor AS do
            WHERE ds.collection_id = c.id
            AND ds.donor_id = do.id");
    }

    if(!$res) {
        echo "Error: $mysqli->error";
        return;
    }
?>
<script>
$(document).ready(function() {
    $("#dose-list").DataTable();

    $("#newDoseForm").submit(function(event) {
        var action = "?method=list-doses&id=";
        $.ajax({
            type: "POST",
            url: "submit.php",
            data: $("#newDoseForm").serialize(),
            success: function(data) {
                if(data) {
                    var res = jQuery.parseJSON(data);
                    if(res.error) {
                        $("#newDoseError").html(res.error);
                    } else {
                        window.location.replace(action + res.id);
                    }
                }
            }
        });

        event.preventDefault();
    });
});
</script>
<?php
    if($level > 0) {
?>
<button style="float: right" type="button" class="btn btn-primary btn-lg" data-toggle="modal" data-target="#newDose">New dose</button>
<div class="modal fade" id="newDose" tabindex="-1" data-backdrop="static" role="dialog" aria-labelledby="newDoseLabel">
  <div class="modal-dialog" role="document">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
        <h4 class="modal-title" id="newDoseLabel">Add a new dose</h4>
      </div>
      <div class="modal-body">
        <form id="newDoseForm" class="form-horizontal">
          <input type="hidden" name="insertType" value="dose">
          <div class="form-group required">
            <label for="doseCollection" class="col-sm-2 control-label">Collection</label>
            <div class="col-sm-10">
                <select class="form-control selectpicker" data-live-search="true" name="doseCollection" id="doseCollection">
<?php
    $samples = $mysqli->query("
        SELECT CONCAT(d.name, ' ', d.surname) AS dn, d.personal_id,
               c.collection_date,
               c.id AS c_id,
               d.id AS d_id
        FROM donor AS d, collection AS c
        WHERE c.donor_id = d.id
    ");
    while($row = $samples->fetch_assoc()) {
        echo "<option value='" . $row["c_id"] . ":" . $row["d_id"] . "'>"
            . $row["dn"] . " (" . $row["personal_id"] . ") - "
            . $row["collection_date"] . "</option>\n";
    }
?>
                </select>
            </div>
          </div>
          <div class="form-group required">
            <label for="doseSuitable" class="col-sm-2 control-label">Suitable</label>
            <div class="col-sm-10">
                <input type="checkbox" id="doseSuitable" name="doseSuitable">
            </div>
          </div>
          <div class="form-group required">
            <label for="doseSelfEx" class="col-sm-2 control-label">Self-excl.</label>
            <div class="col-sm-10">
                <input type="checkbox" id="doseSelfEx" name="doseSelfEx">
            </div>
          </div>
        </form>
        <div id="newDoseError" style="color: #FF0000">
        </div>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
        <button id="newDoseSubmit" form="newDoseForm" type="submit" class="btn btn-primary">Save changes</button>
      </div>
    </div>
  </div>
</div>
<?php
    }
?>
<h1 class="page-header">Doses</h1>
<div class="table-responsive">
    <table id="dose-list" class="table table-hover table-striped">
        <thead>
        <tr>
            <th>#</th>
            <th>Blood type</th>
            <th>Date</th>
            <th>Self-exclusion</th>
            <th>Suitable</th>
            <?php
                if($level > 0) {
            ?>
            <th>Collection</th>
            <th>Donor</th>
            <?php
                }
            ?>
        </tr>
        </thead>
        <tbody>
<?php
    while($row = $res->fetch_assoc()) {
        echo "<tr>";
        echo "<td>" . $row["id"] . "</td>";
        echo "<td>" . $row["blood_type"] . "</td>";
        echo "<td>" . $row["dose_date"] . "</td>";
        echo "<td>" . ($row["self_exclusion"] ? "Yes" : "No") . "</td>";
        echo "<td>" . ($row["suitable"] ? "Yes" : "No") . "</td>";
        if($level > 0) {
            echo "<td><a href='?method=list-collections&id=" . $row["c_id"] . "'>#"
               . $row["c_id"] . "</a></td>";
            echo "<td><a href='?method=list-donors&id=" . $row["do_id"] . "'>"
            . $row["td"] . "</a></td>";
        }
        echo "</tr>";
    }
?>
    </tbody>
    </table>
</div>
<?php
}

function list_reservations($level) {
    if($level < 1)
        return;
    global $mysqli;
    $id = (isset($_GET["id"]) ? $_GET["id"] : NULL);

    if($id !== NULL) {
        $stmt = $mysqli->prepare("
            SELECT r.id, r.res_date, r.state,
            CONCAT(CONCAT_WS(' ', c.name, c.surname), ' - ', c.address) AS cl,
            CONCAT(b.name, ' - ', b.address) AS ob,
            d.id AS d_id,
            b.id AS b_id,
            c.id AS c_id
            FROM reservation AS r, dose AS d, branch AS b, client AS c
            WHERE r.dose_id = d.id
            AND r.branch_id = b.id
            AND r.client_id = c.id
            AND r.id = ?");
        $stmt->bind_param("i", $id);
        $stmt->execute();
        $res = $stmt->get_result();
    } else {
        $res = $mysqli->query("
            SELECT r.id, r.res_date, r.state,
            CONCAT(CONCAT_WS(' ', c.name, c.surname), ' - ', c.address) AS cl,
            CONCAT(b.name, ' - ', b.address) AS ob,
            d.id AS d_id,
            b.id AS b_id,
            c.id AS c_id
            FROM reservation AS r, dose AS d, branch AS b, client AS c
            WHERE r.dose_id = d.id
            AND r.branch_id = b.id
            AND r.client_id = c.id");
    }

    if(!$res) {
        echo "Error: $mysqli->error";
        return;
    }
?>
<script>
$(document).ready(function() {
    $("#reservation-list").DataTable();

    $("#newReservationForm").submit(function(event) {
        var action = "?method=list-reservations&id=";
        $.ajax({
            type: "POST",
            url: "submit.php",
            data: $("#newReservationForm").serialize(),
            success: function(data) {
                if(data) {
                    var res = jQuery.parseJSON(data);
                    if(res.error) {
                        $("#newReservationError").html(res.error);
                    } else {
                        window.location.replace(action + res.id);
                    }
                }
            }
        });

        event.preventDefault();
    });
});
</script>
<button style="float: right" type="button" class="btn btn-primary btn-lg" data-toggle="modal" data-target="#newReservation">New reservation</button>
<div class="modal fade" id="newReservation" tabindex="-1" data-backdrop="static" role="dialog" aria-labelledby="newReservationLabel">
  <div class="modal-dialog" role="document">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
        <h4 class="modal-title" id="newReservationLabel">Add a new reservation</h4>
      </div>
      <div class="modal-body">
        <form id="newReservationForm" class="form-horizontal">
          <input type="hidden" name="insertType" value="reservation">
          <div class="form-group required">
            <label for="reservationCollection" class="col-sm-2 control-label">Collection</label>
            <div class="col-sm-10">
                <select class="form-control selectpicker" data-live-search="true" name="reservationCollection" id="reservationCollection">
<?php
    $doses = $mysqli->query("
        SELECT CONCAT(do.blood_type, ' ', ds.dose_date) AS dinfo,
            do.branch_id,
            b.name,
            ds.id AS ds_id
        FROM donor AS do, dose AS ds, branch AS b
        WHERE ds.donor_id = do.id
        AND ds.suitable = TRUE
        AND ds.self_exclusion = FALSE
        AND b.id = do.branch_id
        AND ds.id NOT IN (
            SELECT dose_id
            FROM reservation
            WHERE state <> 'CANCELLED'
        )
    ");
    while($row = $doses->fetch_assoc()) {
        echo "<option value='" . $row["ds_id"]
            . ":" . $row["branch_id"] . "'>" . $row["dinfo"]
            . " (" . $row["name"] . ")</option>\n";
    }
?>
                </select>
            </div>
          </div>
          <div class="form-group required">
            <label for="reservationClient" class="col-sm-2 control-label">Client</label>
            <div class="col-sm-10">
                <select class="form-control selectpicker" data-live-search="true" name="reservationClient" id="reservationClient">
<?php
    $doses = $mysqli->query("
        SELECT id, CONCAT_WS(' ', name, surname) AS name, address
        FROM client;
    ");
    while($row = $doses->fetch_assoc()) {
        echo "<option value='" . $row["id"] . "'>" . $row["name"] . " - "
            . $row["address"] . "</option>\n";
    }
?>
                </select>
            </div>
          </div>
        </form>
        <div id="newReservationError" style="color: #FF0000">
        </div>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
        <button id="newReservationSubmit" form="newReservationForm" type="submit" class="btn btn-primary">Save changes</button>
      </div>
    </div>
  </div>
</div>
<h1 class="page-header">Reservations</h1>
<div class="table-responsive">
    <table id="reservation-list" class="table table-hover table-striped">
        <thead>
        <tr>
            <th>#</th>
            <th>Date</th>
            <th>State</th>
            <th>Dose</th>
            <th>Branch</th>
            <th>Client</th>
        </tr>
        </thead>
        <tbody>
<?php
    while($row = $res->fetch_assoc()) {
        echo "<tr>";
        echo "<td>" . $row["id"] . "</td>";
        echo "<td>" . $row["res_date"] . "</td>";
        echo "<td>" . $row["state"] . "</td>";
        echo "<td><a href='?method=list-doses&id=" . $row["d_id"] . "'>#"
           . $row["d_id"] . "</a></td>";
        echo "<td><a href='?method=list-branches&id=" . $row["b_id"] . "'>"
           . $row["ob"] . "</a></td>";
        echo "<td><a href='?method=list-clients&id=" . $row["c_id"] . "'>"
           . $row["cl"] . "</a></td>";
        echo "</tr>";
    }
?>
    </tbody>
    </table>
</div>
<?php
}

?>
