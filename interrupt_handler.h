#ifndef NEBULA_INTERRUPT_HANDLER_H
#define NEBULA_INTERRUPT_HANDLER_H

void register_signal_handlers();

void send_interrupt(int pid);

void send_minimize(int pid);

void send_resume(int pid);

#endif
