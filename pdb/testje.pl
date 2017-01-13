#!/usr/bin/perl

use DBI;

$conn = Pg::setdb("", "", "", "", "srobyns");
if ($conn->status != 0)
{
    die "Failed to open db";
}

$result = $conn->exec("insert into data_points values (3, '13/03/2000', '19:20');");
if ($result->resultStatus == 0)
{
    die "Failed to insert";
}

$result = $conn->exec("select * from data_points;");
if ($result->resultStatus == 0)
{
    die "Failed to insert";
}
$result->printTuples(stdout, false, false, 80);
