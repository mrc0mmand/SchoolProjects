<!-- Create a new table dialog -->
<div id="createTable" class="modal fade" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal">&times;</button>
        <h4 class="modal-title">Create table</h4>
      </div>
      <div class="modal-body">
        <form class="form-horizontal" id="createTableForm" method="POST">
          <div class="form-group">
            <label for="tableName" class="control-label col-sm-2">Name</label>
            <div class="col-sm-10">
              <input id="tableName" type="text" name="tableName" class="form-control" placeholder="Table name" required>
            </div>
          </div>
<?php
for($i = 1; $i < 5; $i++) {
    echo <<< EOT
                 <div class="form-group">
                  <label class="control-label col-sm-2">Column $i</label>
                  <div class="col-sm-4">
                    <input type="text" name="table[colname][]" class="form-control" placeholder="Column name">
                  </div>
                  <div class="col-sm-4">
                    <input type="text" name="table[coltype][]" class="form-control" placeholder="Column type">
                  </div>
                </div>
EOT;
}
?>
        <input type="hidden" name="createTable" type="text" value="1">
        <button type="button" class="btn btn-success">+ row</button>
        </form>
      </div>
      <div class="modal-footer">
        <button form="createTableForm" class="btn btn-primary" type="submit">Create table</button>
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
      </div>
    </div>
  </div>
</div>

<!-- Execute a statement dialog -->
<div id="execStmt" class="modal fade" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal">&times;</button>
        <h4 class="modal-title">Execute</h4>
      </div>
      <div class="modal-body">
        <form class="form-horizontal" id="executeStmtForm" method="POST">
          <div class="form-group">
            <label for="statement" class="control-label col-sm-2">Statement</label>
            <div class="col-sm-10">
              <textarea id="statement" name="statement" class="form-control" required></textarea>
            </div>
          </div>
        </form>
      </div>
      <div class="modal-footer">
        <button form="executeStmtForm" class="btn btn-primary" type="submit">Execute</button>
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
      </div>
    </div>
  </div>
</div>

<!-- Settings dialog -->
<div id="settings" class="modal fade" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal">&times;</button>
        <h4 class="modal-title">Settings</h4>
      </div>
      <div class="modal-body">
        <form class="form-horizontal" id="settingsForm" method="POST">
          <div class="form-group">
            <label for="database" class="control-label col-sm-2">Database</label>
            <div class="col-sm-10">
              <select id="database" name="database" class="form-control">
<?php
$res = $mysqli->query("SHOW DATABASES");
while($row = $res->fetch_assoc()) {
    if($row["Database"] == $db) {
        echo "<option selected=\"true\">" . $row["Database"] . "</option>";
    } else {
        echo "<option>" . $row["Database"] . "</option>";
    }
}
?>
              </select>
            </div>
            <label for="charset" class="control-label col-sm-2">Charset</label>
            <div class="col-sm-10">
              <select id="charset" name="charset" class="form-control selectpicker" data-live-search="true">
<?php
$res = $mysqli->query("SHOW CHARACTER SET");
while($row = $res->fetch_assoc()) {
    echo '<option value="' . $row["Charset"] . '">' . $row["Description"] . '</option>';
}
?>
              </select>
            </div>
          </div>
        </form>
      </div>
      <div class="modal-footer">
        <button form="executeStmtForm" class="btn btn-primary" type="submit">Save</button>
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
      </div>
    </div>
  </div>
</div>

<!-- Account dialog -->
<div id="account" class="modal fade" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal">&times;</button>
        <h4 class="modal-title">Account</h4>
      </div>
      <div class="modal-body">
        <form class="form-horizontal" id="accountForm" method="POST">
          <div class="form-group">
            <label for="passwordOld" class="control-label col-sm-3">Old password</label>
            <div class="col-sm-9">
                <input type="password" id="passwordOld" name="passwordOld" class="form-control" placeholder="Old password" required>
            </div>
            <label for="passwordNew1" class="control-label col-sm-3">New password</label>
            <div class="col-sm-9">
                <input type="password" id="passwordNew1" name="passwordNew1" class="form-control" placeholder="New password" required>
            </div>
            <label for="passwordNew2" class="control-label col-sm-3">Repeat password</label>
            <div class="col-sm-9">
                <input type="password" id="passwordNew2" name="passwordNew2" class="form-control" placeholder="Repeat password" required>
            </div>
          </div>
        </form>
      </div>
      <div class="modal-footer">
        <button form="executeStmtForm" class="btn btn-primary" type="submit">Save</button>
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
      </div>
    </div>
  </div>
</div>

<!-- Drop table dialog -->
<div id="dropTable" class="modal fade" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal">&times;</button>
        <h4 class="modal-title">Drop table</h4>
      </div>
      <div class="modal-body">
        Do you really want to drop table <span id="dropTableName"><strong></strong></span>?
      </div>
      <div class="modal-footer">
        <button class="btn btn-danger" type="submit">Drop table</button>
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
      </div>
    </div>
  </div>
</div>

<!-- Edit table dialog -->
<div id="editTable" class="modal fade" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal">&times;</button>
        <h4 class="modal-title">Edit table</h4>
      </div>
      <div class="modal-body">
        <form class="form-horizontal" id="editTableForm" method="POST">
          <div class="form-group">
            <div class="form-group col-sm-12">
              <div class="col-sm-6">
                <label class="control-label">Type</label>
              </div>
              <div class="col-sm-6">
                <label class="control-label">Name</label>
              </div>
            </div>
<?php
$res = $mysqli->query("DESCRIBE " . $_GET["table"]);
if(!$res) {
    echo "Error: $mysqli->error";
    return;
}

while($row = $res->fetch_assoc()) {
    echo '<div class="form-group col-sm-12">';
    echo '<div class="col-sm-6"><input type="text" name="fieldType" value="' . $row["Type"] . '" class="form-control"></div>';
    echo '<div class="col-sm-6"><input type="text" name="fieldName" value="' . $row["Field"] . '" class="form-control"></div>';

    echo "</div>";
}
?>
          </div>
        </form>
      </div>
      <div class="modal-footer">
        <button class="btn btn-danger" type="submit">Save</button>
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
      </div>
    </div>
  </div>
</div>

<!-- Insert a new row dialog -->
<div id="insertRow" class="modal fade" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal">&times;</button>
        <h4 class="modal-title">Insert</h4>
      </div>
      <div class="modal-body">
        <form class="form-horizontal" id="insertRowTable" method="POST">
          <div class="form-group">
<?php
$res = $mysqli->query("DESCRIBE " . $_GET["table"]);
if(!$res) {
    echo "Error: $mysqli->error";
    return;
}

while($row = $res->fetch_assoc()) {
    echo '<div class="form-group col-sm-12">';
    echo '<div class="col-sm-4"><label class="control-label">' . $row["Field"] .
        '</lable></div>';
    echo '<div class="col-sm-8"><input type="text" name="insertValue" placeholder="' .
        $row["Type"] . '" class="form-control"></div>';

    echo "</div>";
}
?>
          </div>
        </form>
      </div>
      <div class="modal-footer">
        <button class="btn btn-primary" type="submit">Insert</button>
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
      </div>
    </div>
  </div>
</div>
