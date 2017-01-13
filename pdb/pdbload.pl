#!/usr/bin/perl

#
# Classes
#

use DBI;

#
# Global data
#

$dbh;
$new_system_index_sth;
$new_workload_index_sth;
$new_processgroup_index_sth;
$the_day = "";

#
# Subroutines .....
#

sub make_time
{
    local $hour = shift;

    if ($hour < 10)
    {
	return "0$hour:00";
    }
    else
    {
	return "$hour:00";
    }
}

sub init_db
{
    $dbh = DBI->connect("dbi:Pg:dbname=srobyns", "", "")
	|| die "Failed to open"; 

    $new_system_index_sth = $dbh->prepare("select max(index) from datapoints;");
    $new_workload_index_sth = $dbh->prepare("select max(index) from workloads;");
    $new_processgroup_index_sth = $dbh->prepare("select max(index) from processgroups;");
}

sub end_db
{
    $dbh->disconnect;
    undef $dbh;
}

sub find_datapoint
{
    local $date = shift;
    local $time = shift;
    local $datapoint;

    $system_index_sth = $dbh->prepare("select index from datapoints where intv_date = '$date' and intv_time = '$time';");
    
    $rv = $system_index_sth->execute();
    if ($rv > 0)
    {
	$statement = $system_index_sth->fetch();
	$datapoint = $statement->[0];
	$system_index_sth->finish();
    }
    else
    {
	$system_index_sth->finish();
	if ($new_system_index_sth->execute() != 1)
	{
	    die "Failed to select max index.";
	}
	$statement = $new_system_index_sth->fetch();
	# $new_system_index_sth->bind_columns(undef, $datapoint);
	$datapoint = $statement->[0] + 1;
	if ($dbh->do("insert into datapoints values ($datapoint, '$date', '$time');")
	    != 1)
	{
	    die "Failed to insert $date $time.\n";
	}
	$new_system_index_sth->finish();
    }

    return $datapoint;
}

sub find_node
{
    local $node_name = shift;
    return 1;
}

sub find_workload
{
    local $workload_name = shift;
    local $workload;

    $workload_index_sth = $dbh->prepare("select index from workloads where workload_name = '$workload_name';");
    
    $rv = $workload_index_sth->execute();
    if ($rv > 0)
    {
	$statement = $workload_index_sth->fetch();
	$workload = $statement->[0];
	$workload_index_sth->finish();
    }
    else
    {
	$workload_index_sth->finish();
	if ($new_workload_index_sth->execute() != 1)
	{
	    die "Failed to select max index.";
	}
	$statement = $new_workload_index_sth->fetch();
	$workload = $statement->[0] + 1;
	if ($dbh->do("insert into workloads values ($workload, '$workload_name');")
	    != 1)
	{
	    die "Failed to insert $workload_name.\n";
	}
	$new_workload_index_sth->finish();
    }

    return $workload;
}

sub find_processgroup
{
    local $processgroup_name = shift;
    local $processgroup;

    $processgroup_index_sth = $dbh->prepare("select index from processgroups where processgroup_name = '$processgroup_name';");
    
    $rv = $processgroup_index_sth->execute();
    if ($rv > 0)
    {
	$statement = $processgroup_index_sth->fetch();
	$processgroup = $statement->[0];
	$processgroup_index_sth->finish();
    }
    else
    {
	$processgroup_index_sth->finish();
	if ($new_processgroup_index_sth->execute() != 1)
	{
	    die "Failed to select max index.";
	}
	$statement = $new_processgroup_index_sth->fetch();
	$processgroup = $statement->[0] + 1;
	if ($dbh->do("insert into processgroups values ($processgroup, '$processgroup_name');")
	    != 1)
	{
	    die "Failed to insert $processgroup_name.\n";
	}
	$new_processgroup_index_sth->finish();
    }

    return $processgroup;
}

sub store_system_data
{
    local $hostname = shift;
    local $date = shift;
    local $time = shift;
    local $user_cpu = shift;
    local $system_cpu = shift;
    local $node = find_node($hostname);
    local $datapoint = find_datapoint($date, $time);

    $dbh->do("insert into systemdata values ($datapoint, $node, $user_cpu, $system_cpu);");

}

sub store_workload_data
{
    local $hostname = shift;
    local $date = shift;
    local $time = shift;
    local $workload_name = shift;
    local $processgroup_name = shift;
    local $user_cpu = shift;
    local $system_cpu = shift;
    local $node = find_node($hostname);
    local $datapoint = find_datapoint($date, $time);
    local $workload = find_workload($workload_name);
    local $processgroup = find_processgroup($processgroup_name);

    $dbh->do("insert into workloaddata values ($datapoint, $node, $workload, $processgroup, $user_cpu, $system_cpu);");

}

sub system_data
{
    local @line;

    for ($hour = 0; $hour < 24; $hour++)
    {
	$start_hour = make_time($hour);
	$end_hour = make_time($hour+1);
	open(INPUT, "runq report -CS -s$start_hour -e$end_hour|")
	    || die "failed to open pipe to runq";
	while ( <INPUT> )
	{
	    next if (/Date/);
	    @line = split(';');
	    if ($the_day == "")
	    {
		$the_day = $line[0];
	    }
	    store_system_data("localhost", $line[0], $line[1],
			      $line[2] + $line[3], $line[4]);
	}
	close INPUT;
    }
}

sub workload_data
{
    local $hour;
    local $start_hour;
    local $end_hour;
    local @line;

    for ($hour = 0; $hour < 24; $hour++)
    {
	$start_hour = make_time($hour);
	$end_hour = make_time($hour+1);
	open(INPUT, "runq analyze -C -s$start_hour -e$end_hour |")
	    || die "failed to open pipe to runq";
	while ( <INPUT> )
	{
	    next if (/WorkLoad/);
	    @line = split(';');
	    store_workload_data("localhost", $the_day, $start_hour,
				$line[0], $line[1],
				$line[3], $line[4]);
	}
	close INPUT;
    }
}


#
# Main entry point
#

init_db();

system_data();
workload_data();

end_db();

