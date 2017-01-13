create table datapoints (
       index		integer not null unique,
       intv_date	date not null,
       intv_time	time not null);

create table systems(
       index		integer not null unique,
       name		char(20) not null);

create table systemdata(
       datapoint	integer not null,
       system		integer not null,
       cpu_user		float not null,
       cpu_system	float not null);

create table workloads(
       index		integer not null unique,
       workload_name	char(20));

create table processgroups(
       index		integer not null unique,
       processgroup_name	char(20));

create table workloaddata(
       datapoint	integer not null,
       system		integer not null,
       workload		integer not null,
       processgroup	integer not null,
       cpu_user		float not null,
       cpu_system	float not null);
