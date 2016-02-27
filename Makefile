all: reset process gen sched msg

process:
	cc process.c -o process.out

msg:
	ipcrm -a

reset:
	reset

gen:
	cc gen.c -o gen.out

sched:
	cc sched.c -o sched.out

clean: clean_sched clean_gen clean_process

clean_gen:
	rm gen.out

clean_sched:
	rm sched.out

clean_process:
	rm process.out
