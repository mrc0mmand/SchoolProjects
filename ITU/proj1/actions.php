<?php

function list_table($table_name) {
    global $mysqli;

    # Safety first
    $res = $mysqli->query("SELECT * FROM " . $table_name);
    if(!$res) {
        echo "Error: $mysqli->error";
        return;
    }

    $row = $res->fetch_assoc();
?>

<script>
$(document).ready(function() {
    $("#table-list").DataTable();

    $("#table-list tr").click(function() {
        var triggered = $(this);
        $("#tableRowEditContent").empty();
        $("#table-list th:gt(0)").each(function(index, elem) {
            var rname = $(this).text();
            var rid = "col_" + rname;
            $("#tableRowEditContent").append(
'<label for="' + rid +  '" class="control-label col-sm-3">' + rname + '</label>' +
    '<div class="col-sm-9">' +
        '<input type="text" id="' + rid + '" name="' + rid + '" class="form-control" value="' + $(triggered).find("td").eq($(this).index()).text() + '">' +
                 '</div>'
             );
         });

        $("#tableRowEdit").modal("show");
    });
});
</script>
<!-- Edit dialog -->
<div id="tableRowEdit" class="modal fade" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal">&times;</button>
        <h4 class="modal-title">Edit row</h4>
      </div>
      <div class="modal-body">
        <form class="form-horizontal" id="accountForm" method="POST">
          <div id="tableRowEditContent" class="form-group">
            <!-- Form content -->
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
<div class="table-responsive">
    <table id="table-list" class="table table-hover table-striped">
        <thead>
        <tr>
<?php
    echo '<th></th>';
    foreach(array_keys($row) as $key) {
        echo '<th>' . $key . '</th>';
    }
?>
        </tr>
        </thead>
        <tbody>
<?php
    while($row) {
        echo "<tr>";
        echo '<td><input type="checkbox"></td>';
        foreach($row as $item) {
            echo "<td>" . $item  . "</td>";
        }
        echo "</tr>";
        $row = $res->fetch_assoc();
    }
?>

    </tbody>
    </table>
</div>
<div>
    <select id="tableActionSelect" class="selectpicker">
        <option value="delete">Delete selected rows</option>
        <option value="edit">Edit selected rows</option>
        <option value="export">Export selected rows</option>
    </select>
    <button class="btn btn-primary">Select</button>
</div>
<?php
}
?>
