<?php

include "db.php";
include "utils.php";

$data = array("error" => "");

if(isset($_POST["insertType"])) {
    switch($_POST["insertType"]) {
    case "branch":
        $stmt = $mysqli->prepare("
            INSERT INTO branch(name, address) VALUES(?, ?)");
        $stmt->bind_param("ss", $_POST["branchName"], $_POST["branchAddress"]);
        if(!$stmt->execute()) {
            $data["error"] = "SQL error: " . $mysqli->error;
        } else {
            $data["id"] = $mysqli->insert_id;
        }

        break;
    case "donor":
        $stmt = $mysqli->prepare("
            INSERT INTO donor(name, surname, sex, weight, birth_date,
                personal_id, address, insurance, blood_type, reg_date,
                branch_id)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, NOW(), ?)
        ");
        $stmt->bind_param("sssisssssi", $_POST["donorName"],
            $_POST["donorSurname"], $_POST["donorSex"], $_POST["donorWeight"],
            $_POST["donorBirth"], $_POST["donorPersonalID"],
            $_POST["donorAddress"], $_POST["donorInsurance"],
            $_POST["donorBlood"], $_POST["donorBranch"]);
        if(!$stmt->execute()) {
            $data["error"] = "SQL error: " . $mysqli->error;
        } else {
            $data["id"] = $mysqli->insert_id;
        }

        break;
    case "client":
        $stmt = $mysqli->prepare("
            INSERT INTO client(type, name, surname, address, branch_id)
            VALUES (?, ?, ?, ?, ?)
        ");
        $stmt->bind_param("ssssi", $_POST["clientType"], $_POST["clientName"],
            $_POST["clientSurname"], $_POST["clientAddress"],
            $_POST["clientBranch"]);
        if(!$stmt->execute()) {
            $data["error"] = "SQL error: " . $mysqli->error;
        } else {
            $data["id"] = $mysqli->insert_id;
        }

        break;
    case "invitation":
        $stmt = $mysqli->prepare("
            INSERT INTO invitation(inv_date, content, state, donor_id,
                            branch_id)
            VALUES (NOW(), ?, 'SENT', ?, ?)
        ");

        if(!isset($_POST["invitationName"])) {
            $data["error"] = "Missing donor name";
            goto end;
        }

        $ids = explode(":", $_POST["invitationName"]);
        if(count($ids) != 2) {
            $data["error"] = "Invalid donor/branch ID";
            goto end;
        }

        $stmt->bind_param("sii", $_POST["invitationText"], $ids[0], $ids[1]);
        if(!$stmt->execute()) {
            $data["error"] = "SQL error: " . $mysqli->error;
        } else {
            $data["id"] = $mysqli->insert_id;
        }

        break;
    case "sample":
        $stmt = $mysqli->prepare("
            INSERT INTO sample(cbc, suitable, reason, sample_date, donor_id)
            VALUES (?, ?, ?, NOW(), ?)
        ");
        $suitable = isset($_POST["sampleSuitable"]);
        $stmt->bind_param("sisi", $_POST["sampleCBC"], $suitable,
                $_POST["sampleReason"], $_POST["sampleName"]);
        if(!$stmt->execute()) {
            $data["error"] = "SQL error: " . $mysqli->error;
        } else {
            $data["id"] = $mysqli->insert_id;
        }

        break;
    case "collection":
        $stmt = $mysqli->prepare("
            INSERT INTO collection(collection_date, type, complications,
                    collection_done, donor_id, sample_id)
            VALUES (NOW(), ?, ?, ?, ?, ?)
        ");

        if(!isset($_POST["collectionSample"])) {
            $data["error"] = "Missing sample";
            goto end;
        }

        $ids = explode(":", $_POST["collectionSample"]);
        if(count($ids) != 2) {
            $data["error"] = "Invalid donor/sample ID";
            goto end;
        }

        $cdone = isset($_POST["collectionSuccessful"]);
        $stmt->bind_param("ssiii", $_POST["collectionType"],
                $_POST["collectionComplications"], $cdone, $ids[0], $ids[1]);
        if(!$stmt->execute()) {
            $data["error"] = "SQL error: " . $mysqli->error;
        } else {
            $data["id"] = $mysqli->insert_id;
        }

        break;
    case "dose":
        $stmt = $mysqli->prepare("
            INSERT INTO dose(dose_date, self_exclusion, suitable, collection_id,
                    donor_id)
            VALUES(NOW(), ?, ?, ?, ?)
        ");

        if(!isset($_POST["doseCollection"])) {
            $data["error"] = "Missing sample";
            goto end;
        }

        $ids = explode(":", $_POST["doseCollection"]);
        if(count($ids) != 2) {
            $data["error"] = "Invalid donor/sample ID";
            goto end;
        }

        $selfex = isset($_POST["doseSelfEx"]);
        $suitable = isset($_POST["doseSuitable"]);
        $stmt->bind_param("iiii", $selfex, $suitable, $ids[0], $ids[1]);
        if(!$stmt->execute()) {
            $data["error"] = "SQL error: " . $mysqli->error;
        } else {
            $data["id"] = $mysqli->insert_id;
        }

        break;
    case "reservation":
        $stmt = $mysqli->prepare("
            INSERT INTO reservation(res_date, state, dose_id, branch_id,
                client_id)
            VALUES(NOW(), 'NEW', ?, ?, ?)
        ");

        if(!isset($_POST["reservationCollection"])) {
            $data["error"] = "Missing dose";
            goto end;
        }

        $ids = explode(":", $_POST["reservationCollection"]);
        if(count($ids) != 2) {
            $data["error"] = "Invalid dose/branch ID";
            goto end;
        }

        $stmt->bind_param("iii", $ids[0], $ids[1], $_POST["reservationClient"]);
        if(!$stmt->execute()) {
            $data["error"] = "SQL error: " . $mysqli->error;
        } else {
            $data["id"] = $mysqli->insert_id;
        }

        break;
    case "user":
        $stmt = $mysqli->prepare("
            INSERT INTO users(username, email, password, level)
            VALUES (?, ?, ?, ?)
        ");

        if($_POST["userPass1"] !== $_POST["userPass2"]) {
            $data["error"] = "Passwords do not match";
            goto end;
        }

        $pass = hash_pass($_POST["userPass1"]);
        $stmt->bind_param("sssi", $_POST["userName"], $_POST["userEmail"],
                    $pass, $_POST["userLevel"]);
        if(!$stmt->execute()) {
            $data["error"] = "SQL error: " . $mysqli->error;
        } else {
            $data["id"] = $mysqli->insert_id;
        }

        break;
    case "user-update":
        if(isset($_POST["userPass1"]) && isset($_POST["userPass2"])) {
            if($_POST["userPass1"] !== $_POST["userPass2"]) {
                $data["error"] = "Passwords do not match";
                goto end;
            }

            $pass = hash_pass($_POST["userPass1"]);
            $stmt = $mysqli->prepare("
                UPDATE users
                SET username = ?, email = ?, level = ?, password = ?
                WHERE id = ?
            ");
            $stmt->bind_param("ssisi", $_POST["userName"], $_POST["userEmail"],
                $_POST["userLevel"], $pass, $_POST["userID"]);
        } else {
            $stmt = $mysqli->prepare("
                UPDATE users
                SET username = ?, email = ?, level = ?
                WHERE id = ?
            ");
            $stmt->bind_param("ssii", $_POST["userName"], $_POST["userEmail"],
                $_POST["userLevel"], $_POST["userID"]);
        }

        if(!$stmt->execute()) {
            $data["error"] = "SQL error: " . $mysqli->error;
        } else {
            $data["id"] = $_POST["userID"];
        }
        break;
    default:
        $data["error"] = "Invalid type";
        break;
    }
} else {
    $data["error"] = "Missing data";
}

end:
echo json_encode($data);

?>
