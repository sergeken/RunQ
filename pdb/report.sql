select
	intv_date, intv_time, workload_name, processgroup_name,
	cpu_user, cpu_system
from
		datapoints, workloads, processgroups, workloaddata
where
	datapoints.index = workloaddata.datapoint and
	workloads.index = workloaddata.workload and
	processgroups.index = workloaddata.processgroup
;
