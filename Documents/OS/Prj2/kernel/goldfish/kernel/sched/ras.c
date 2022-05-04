#include "sched.h"

#include <linux/slab.h>

static unsigned int get_timeslice(struct rq *rq, struct task_struct *task);

static inline struct task_struct *ras_task_of(struct sched_ras_entity *ras_se){
    return container_of(ras_se, struct task_struct, ras);
}

static inline struct rq *rq_of_ras_rq(struct ras_rq *ras_rq){
    return container_of(ras_rq, struct rq, ras);
}

static inline struct ras_rq *ras_rq_of_se(struct sched_ras_entity *ras_se){
    struct task_struct *p = ras_task_of(ras_se);
    struct rq *rq = task_rq(p);

    return &rq->ras;
}

static inline int on_ras_rq(struct sched_ras_entity *ras_se){
    return !list_empty(&ras_se->run_list);
}

static void update_curr_ras(struct rq *rq) {
    struct task_struct *curr = rq->curr;
    struct sched_ras_entity *ras_se = &curr->ras;
    struct ras_rq *ras_rq = ras_rq_of_se(ras_se);
    u64 delta_exec;

    if(curr->sched_class != &ras_sched_class) return;

    delta_exec = rq->clock_task - curr->se.exec_start;
    if(unlikely((s64)delta_exec < 0)) delta_exec = 0;

	schedstat_set(curr->se.statistics.exec_max,
		      max(curr->se.statistics.exec_max, delta_exec));

    curr->se.sum_exec_runtime += delta_exec;
    account_group_exec_runtime(curr, delta_exec);

    curr->se.exec_start = rq->clock_task;
    cpuacct_charge(curr, delta_exec);
    /*
    GROUP group_sched related!
    */

    raw_spin_lock(&ras_rq->ras_runtime_lock);
    ras_rq->ras_runtime += delta_exec;
    raw_spin_unlock(&ras_rq->ras_runtime_lock);
}

static void requeue_ras_entity(struct ras_rq *ras_rq, struct sched_ras_entity *ras_se, int head){
    
}

static void requeue_task_ras(struct rq *rq, struct task_struct *p, int head){
    struct sched_ras_entity ras_se = &p->ras;
    struct ras_rq = &rq->ras;

    requeue_ras_entity(ras_rq, ras_se, head);
}

static void yield_task_ras(struct rq *rq){
    requeue_task_ras(rq, rq->curr, 0);
    printk(KERN_INFO "task %d yield!\n", rq->curr->pid);
}

static void check_preempt_curr_ras(struct rq *rq, struct task_struct *p){
    if(p->prio < rq->curr->prio){
        resched_task(rq->curr);
        printk(KERN_INFO "task %d preempt %d\n",p->pid, rq->curr->pid);
        return;
    }
}

static struct sched_ras_entity *pick_next_ras_entity(struct rq *rq, struct ras_rq *ras_rq){
    struct list_head head = &ras_rq->ras_run_list;
    struct sched_ras_entity *next = NULL;
    next = list_entry(queue->next, struct sched_ras_entity, run_list);

    return next;
}

static struct task_struct *pick_next_task_ras(struct rq *rq){
    struct sched_ras_entity *ras_se;
    struct task_struct *p;
    struct ras_rq *ras_rq;

    ras_rq = &rq->ras;
    if(!ras_rq->ras_nr_running) return NULL;
    /*GROUP*/

    ras_se = pick_next_ras_entity(rq, ras_rq);

    BUG_ON(!ras_se);
    
    p = ras_task_of(ras_se);
    if(p){
        printk(KERN_INFO "Pick next task: %d\n", p->pid);
    }
    else{
        printk(KERN_INFO "task is NULL | pick_next_task_ras!\n");
        return NULL;
    }
    p->se.exec_start = rq->clock_task;
    

    return p;
}

static void put_prev_task_ras(struct rq *rq, struct task_struct *p){
    update_curr_ras(rq);

    /*QUESTION*/
    if(on_ras_rq(&p->ras)) requeue_task_ras(rq, p, 0);
    printk(KERN_INFO "Put prev task: %d/n", p->pid);
}

static void switched_to_ras(struct rq * rq, struct task_struct *p){
    /*PRIO*/
}

static void prio_changed_ras(struct rq *rq, struct task_struct *p, int oldprio){
    /*PRIO*/
}

static void task_tick_ras(struct rq *rq, struct task_struct *p, int queued){
    struct sched_ras_entity *ras_se = &p->ras;
    update_curr_ras(rq);

    //watchdog(rq, p); GROUP group sched related!

    if(--p->ras.time_slice){
        printk(KERN_INFO "remaining time_slice: %u\n", p->ras.time_slice);
        return;
    }

    p->ras.time_slice = get_timeslice(rq, p);
    printk(KERN_INFO "new time_slice: %u\n", p->ras.time_slice);
    if(ras_se->run_list.prev != ras_se.run_list.next){
        requeue_task_ras(rq, p, 0);
        set_tsk_need_resched(p);
    }
    /*
    GROUP group_sched related!
    */
}

static void set_curr_task_ras(struct rq *rq){
    struct task_struct *p = rq->curr;

    p->se.exec_start = rq->clock_task;
}

static unsigned int get_timeslice(struct rq *rq, struct task_struct *task){
    /*
    TODO!
    */
}

static unsigned int get_prob(struct rq *rq, struct task_struct *task){
    /*
    TODO!
    */
}

/*
If something went wrong, return -1.
*/
static unsigned int get_rr_interval_ras(struct rq *rq, struct task_struct *task){
    if(!rq || !task){
        printk(KERN_INFO "rq or task is NULL | get_rr_interval_ras");
        return -1;
    }

    unsigned int time_slice;
    
    time_slice = get_timeslice(rq, task);
    if(time_slice < RAS_TIMESLICE_MIN || time_slice > RAS_TIMESLICE_MAX){
        printk(KERN_INFO "time slice out of range | get_rr_interval_ras");
        return -1;
    }
    printk(KERN_INFO "get_rr_interval_ras of task %d is: %u\n", task->pid, time_slice);
    return time_slice;
}

const struct sched_class ras_sched_class = {
	.next			= &idle_sched_class,
	.enqueue_task		= enqueue_task_ras,
	.dequeue_task		= dequeue_task_ras,
	.yield_task		= yield_task_ras,

	.check_preempt_curr	= check_preempt_curr_ras,

	.pick_next_task		= pick_next_task_ras,
	.put_prev_task		= put_prev_task_ras,

#ifdef CONFIG_SMP
	.select_task_rq		= select_task_rq_ras,

	.set_cpus_allowed       = set_cpus_allowed_ras,
	.rq_online              = rq_online_ras,
	.rq_offline             = rq_offline_ras,
	.pre_schedule		= pre_schedule_ras,
	.post_schedule		= post_schedule_ras,
	.task_woken		= task_woken_ras,
	.switched_from		= switched_from_ras,
#endif

	.set_curr_task          = set_curr_task_ras,
	.task_tick		= task_tick_ras,

	.get_rr_interval	= get_rr_interval_ras,

	.prio_changed		= prio_changed_ras,
	.switched_to		= switched_to_ras,
};