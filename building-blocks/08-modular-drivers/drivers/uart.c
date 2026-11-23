// uart.c
// Include necessary headers (avr/io, stdio, uart.h)
// Define F_CPU and BAUD, and calculate UBRR_VAL
#include <avr/io.h>
#include <stdio.h>
#include "uart.h"

#define F_CPU 16000000UL
#define BAUD 9600
#define UBRR_VAL ((F_CPU / (16UL * BAUD)) - 1)

// Function to be linked to stdout for printf.
static int uart_putchar(char c, FILE *stream) {
  // Wait for the UDRE0 flag in UCSR0A to be set.
  if (c == '\n'){
    uart_putchar('\r', stream); // Send CR before LF
  }
  // Write the character 'c' to the UDR0 register.
  while(!(UCSR0A & (1 << UDRE0)));
  UDR0 = c;
  return 0;
}
// Create the FILE stream object linked to uart_putchar
static FILE uart_stream = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

void uart_init(void) {
  // Set baud rate
  UBRR0H = (uint8_t)(UBRR_VAL >> 8);
  UBRR0L = (uint8_t)(UBRR_VAL);

  // Enable transmitter, receiver, and RX Complete Interrupt
  UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);

  // Frame format: 8 data bits, 1 stop bit, no parity
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}
// Implement uart_get_stream to return a pointer to your stream object.
FILE* uart_get_stream(void) {
  return &uart_stream;
}