/*
 * Swarthmore College, CS 31
 * Copyright (c) 2022 Swarthmore College Computer Science Department,
 * Swarthmore PA, Professor Kevin Webb
 */

/* CS31 Lab 2 Part 2 Solution
   Michael Lu
*/
#include <stdio.h>

/* This functions answers Q1: how many different representations
 * can printf display the value 97?
 */
void question1() {
  int val;
  val = 97;

  printf("\n**** Question 1 ****\n");
  printf("My answer to question 1 is: printf can display the value 97 as 3 different\nrepresentations:\n");
  printf("  as a decimal  = %d \n", val);
  printf("  as a octal  = %o \n", val);
  printf("  as a hexadecimal  = %x \n", val);

}

/* This functions answers Q2: What is the maximum value that can be 
 * stored in a C unsigned int variable (unsigned)?
 */
void question2() {
  unsigned int answer;
  answer = 4294967295;
  printf("\n**** Question 2 ****\n");
  printf("My answer to question 2 is: the maximum value is %u, which is 0x%x\n", answer, answer);
  printf("If we increase the maximum value by one, it will go back to 0 because there is\n  an overflow.\n");
  printf("Here is an example:\n\n");  
  printf("  The oringal number: %u.\n",answer);
  printf("  The increased one number: %u.\n\n",answer+1);
  printf("Thus, if we want store a value in a unsigned int, the maximum value will be \n%u so that there's no overflow.\n", answer);

}

/* This functions answers Q3: What is the maximum positive value that can be 
 * stored in a C int variable (signed)?
 */
void question3() {
  signed int answer;
  unsigned int x = 4294967295;
  answer = 2147483647;
  printf("\n**** Question 3 ****\n");
  printf("My answer to question 3 is: the maximum value is %d\n", answer);
  printf("Because like the 2's complement, %d to %u is divided into %d to %d\n  and %d to %d.\n",0,x,0,answer,x,x/2+1);
  printf("If we increase the maximum value by one, it will go to %d because it\n  reaches the negative part of the signed integers:\n",answer+1);
  printf("Here is an example:\n\n");
  printf("    The oringal number: %d.\n",answer);
  printf("    The increased one number: %d.\n\n",answer+1);
  printf("Thus, if we want store a value in a signed int, the maximum value will be \n%d so that there's no overflow.\n", answer);

}

/* This functions answers Q4: What arithmetic operation is equivalent to left 
 * shifting an unsigned int value by 1?
 */
void question4() {
  unsigned int a = 2;
  unsigned int b = 35;
  unsigned int c = 104;
  unsigned int d = 331;
  int left_shift = 1;
  printf("\n**** Question 4 ****\n");
  printf("My answer to question 4 is: it is equivalent to timing the unsigned int value by 2\n");
  printf("Here are three examples:\n\n");
  printf("  The oringal int:   After left shift by %d:   Quotient of left shifted and original:\n",left_shift);
  printf("         %u                   %u                               %u\n",a, a << left_shift, (a << left_shift)/a);
  printf("         %u                  %u                              %u\n",b, b << left_shift, (b << left_shift)/b);
  printf("         %u                 %u                             %u\n",c, c << left_shift, (c << left_shift)/c);
  printf("         %u                 %u                             %u\n\n",d, d << left_shift, (d << left_shift)/d);
  printf("As shown in the examples, quotient of left shifted and original is always two.\n");
  printf("Thus, it means that left shifting an unsigned int value by 1 is equivalent \nto timing the unsigned int value by 2\n");

}

/* This functions answers Q5: What arithmetic operation is equivalent to left 
 * shifting an unsigned int value by 2?
 */
void question5() {
  unsigned int a = 2;
  unsigned int b = 35;
  unsigned int c = 104;
  unsigned int d = 331;
  int left_shift = 2;
  printf("\n**** Question 5 ****\n");
  printf("My answer to question 5 is: it is equivalent to timing the unsigned int value by 4\n");
  printf("Here are three examples:\n\n");
  printf("  The oringal int:   After left shift by %d:   Quotient of left shifted and original:\n",left_shift);
  printf("         %u                   %u                               %u\n",a, a << left_shift, (a << left_shift)/a);
  printf("         %u                  %u                             %u\n",b, b << left_shift, (b << left_shift)/b);
  printf("         %u                 %u                             %u\n",c, c << left_shift, (c << left_shift)/c);
  printf("         %u                 %u                            %u\n\n",d, d << left_shift, (d << left_shift)/d);
  printf("As shown in the examples, quotient of left shifted and original is always four.\n");
  printf("Thus, it means that left shifting an unsigned int value by %d is equivalent \nto timing the unsigned int value by 4\n",left_shift);

}

/* This functions answers Q6: What arithmetic operation is equivalent to right 
 * shifting an unsigned int value by 1?
 */
void question6() {
  unsigned int a = 2;
  unsigned int b = 35;
  unsigned int c = 104;
  unsigned int d = 331;
  int right_shift = 1;
  printf("\n**** Question 6 ****\n");
  printf("My answer to question 6 is: it is equivalent to dividing the unsigned int value by 2\n");
  printf("Here are three examples:\n\n");
  printf("  Oringal int:   After right shift by %d:   Quotient of original and right shifted:\n",right_shift);
  printf("       %u                   %u                                %u\n",a, a >> right_shift, a/(a >> right_shift));
  printf("       %u                  %u                               %u\n",b, b >> right_shift, b/(b >> right_shift));
  printf("       %u                 %u                               %u\n",c, c >> right_shift, c/(c >> right_shift));
  printf("       %u                 %u                              %u\n\n",d, d >> right_shift, d/(d >> right_shift));
  printf("As shown in the examples, quotient of original and right shifted is always two.\n");
  printf("Thus, it means that right shifting an unsigned int value by %d is equivalent \nto dividing the unsigned int value by 2\n",right_shift);

}

int main() {

  printf("%s's solution to Lab 2 Part2:\n", "Michael Lu");

  question1();  // call the question1 function
  question2();  // call the question2 function
  question3();  // call the question3 function
  question4();  // call the question4 function
  question5();  // call the question5 function
  question6();  // call the question6 function


  return 0;
}
