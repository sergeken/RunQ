#!/usr/bin/perl

#
# Global data
#

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


sub store_system_data
{
    local $hostname = shift;
    local $date = shift;
    local $time = shift;
    local $user_cpu = shift;
    local $system_cpu = shift;

    print "$date, $time, $user_cpu, $system_cpu\n";

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

    print "$date, $time, $workload_name, $processgroup_name, $user_cpu, $system_cpu\n";

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

system_data();
workload_data();


