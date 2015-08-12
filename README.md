boost.job
=============

boost.job  provides a framework for asynchronous execution of job-graphs 
(a job is a small unit of code that can be executed independently and
parallel, e.g. it's a fain-grained work item) of arbitrary dependency, e.g.
solving the many-dependent-jobs problem (M: worker threads, N: jobs waiting for
other jobs; M << N).

Each (logical) processor of a dedicated processor set (configurable) gets a worker-thread
assigned. Each worker-thread runs a fiber-pool (launch policy is customizable).
A submitted job is always executed by a worker-fiber. While using boost.fiber
jobs are scheduled cooperatively so that a job can yield to other jobs during its
execution. Jobs can be synchronized via primitives like mutex's, conditions,
barriers and channels (message exchange) from boost.fiber without blocking
the worker-thread. Fibers provide a fast context switch.

boost.job supports to inspect the NUMA topology (e.g. which logical processors are
online, and which processors share L1/L2/L3 cache). This information can be used to
select a appropriate processor for a job.
For a fast access to job related data, boost.job provides functions and
classes to allocate memory on NUMA nodes (processors of a NUMA node share
cache and have a faster access to its associated memory bank than processors of other
nodes.).
