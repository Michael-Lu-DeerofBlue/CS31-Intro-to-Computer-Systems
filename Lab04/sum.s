    .text
    .globl sum
    .type sum, @function
sum:
    # This determines the total size of your stack frame. Here, we
    # are saving 32 bytes in the stack frame, which is definitely
    # overkill for this small example.
    sub sp, sp, #32

    # You should not change anything above here.

    # ---Implement the body of this function below these comments.---
    # The parameter is passed in via register x0.



    # Place your eventual return value in register x0.
    # With the line below, this function always returns 0 right now.
    #n is the user input; res is the return of this ARM code
    #n is stored intitally in x0, n is being copied from register x0 to x1
    mov x1, x0 
    cmp x1, #0 //compare n and 0 
    b.eq .L2   //if equals, jump to label 2
    b.mi .L3   //if negative, jump to label 3; if neither, continues
    .L1:
        sub x1, x1, #1 //subtract n with 1, and store the result in x1
        add x0, x0, x1 //add (n-1) with n and store the result in x0
    cmp x1, #0 //compare (n-1) and 0
    b.eq .L2 //if equals, jump to label 2
    b .L1 //if not, jump to label 1, and go over the loop again
    .L2:
        b .DONE
    .L3:
        mov x0, #-1 //set x0 to -1, thus make res -1
    .DONE:
    
        

    # ---You should not change anything below here.---
    # This restores the stack pointer and returns from the function.
    add sp, sp, #32
    ret
    .size sum, .-sum

