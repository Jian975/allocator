#Author: Xuejian Sundvall
PRINT_STRING = 4
PRINT_INT = 1
PRINT_CHAR = 11
READ_STRING = 8
METADATA_SIZE = 51
NODE_SIZE = 12
MIN_HEAP_SIZE = 4
MAX_HEAP_SIZE = 1000
EXIT = 10

        .data
        .align  2
newline:
        .asciiz "\n"
init_message_front:
        .asciiz "Initializing allocator with "
init_message_back:
        .asciiz " bytes.\n"
left_parentheses:
        .asciiz "("
right_parentheses:
        .asciiz ")"
malloc_message_middle:
        .asciiz " = malloc("
free_message_front:
        .asciiz "free("
double_equals:
        .asciiz " == "
equals:
        .asciiz " = "
size_msg:
        .asciiz "[size="
allocated_msg:
        .asciiz ", allocated="
address_msg:
        .asciiz ", address="
end_bracket:
        .asciiz "]\n"
invalid_arena_msg:
        .asciiz "Invalid arena size, allocator terminating.\n"
bad_free_msg:
        .asciiz "Freeing memory not from malloc\n"
no_memory_msg:
        .asciiz "Out of Memory\n"
        .align  4
heap:
        .space  1000
metadata:
        .space  METADATA_SIZE * NODE_SIZE
metadata_size:
        .word   1
heap_size:
        .word   0
variables:
        .space  26 * 4
buffer:
        .space  128

        .text
        .align  2

        .globl  main

main:
        addi    $sp, $sp, -24
        sw      $ra, 20($sp)
        sw      $s0, 16($sp)
        sw      $s1, 12($sp)
        sw      $s2, 8($sp)
        sw      $s3, 4($sp)
        sw      $s4, 0($sp)

        jal     read

        j       done
 

#a0 - address to find
#v0 - metadata chunk associated with that address, -1 if not found
find:
        addi    $sp, $sp, -24
        sw      $ra, 20($sp)
        sw      $s0, 16($sp)
        sw      $s1, 12($sp)
        sw      $s2, 8($sp)
        sw      $s3, 4($sp)
        sw      $s4, 0($sp)

        move    $s0, $a0                #s0 is the target address
        li      $s1, 0                  #s1 is the index
        la      $s2, metadata_size
        lw      $s2, 0($s2)             #s2 is the list size

find_loop:
        slt     $t0, $s1, $s2
        beq     $t0, $zero, not_found

        #load metadata chunk
        li      $t0, NODE_SIZE
        mul     $t0, $t0, $s1
        la      $t1, metadata
        add     $s3, $t0, $t1           #s3 is current metadata chunk
        
        #chunk address
        lw      $t0, 8($s3)
        beq     $t0, $s0, found
        
        addi    $s1, $s1, 1
        j       find_loop

found:
        move    $v0, $s3
        j       find_done

not_found:
        li      $v0, -1
        j       find_done

find_done:
        lw      $s3, 4($sp)
        lw      $s2, 8($sp)
        lw      $s1, 12($sp)
        lw      $s0, 16($sp)
        lw      $ra, 20($sp)
        addi    $sp, $sp, 24
        jr      $ra


#a0 - memory to free
free:
        addi    $sp, $sp, -24
        sw      $ra, 20($sp)
        sw      $s0, 16($sp)
        sw      $s1, 12($sp)
        sw      $s2, 8($sp)
        sw      $s3, 4($sp)
        sw      $s4, 0($sp)

        move    $s0, $a0                #s0 contains address to free
        beq     $s0, $zero, bad_free

        move    $a0, $s0
        jal     find
        move    $s1, $v0                #s1 is the metadata chunk

        li      $t1, -1
        beq     $s1, $t1, bad_free

        #free this chunk
        sw      $zero, 4($s1)

back_merge:
        #can't back merge if it's last block
        la      $t0, metadata
        sub     $t0, $s1, $t0
        li      $t1, NODE_SIZE
        div     $t1, $t0, $t1           #index of freed chunk

        la      $t0, metadata_size
        lw      $t0, 0($t0)
        addi    $t0, $t0, -1

        slt     $t0, $t1, $t0
        beq     $t0, $zero, front_merge

        #check if next chunk is free
        li      $t0, NODE_SIZE
        add     $s2, $s1, $t0           #s2 is next chunk
        lw      $t0, 4($s2)
        bne     $t0, $zero, front_merge

        #we can back merge
        #merge sizes
        lw      $t0, 0($s1)
        lw      $t1, 0($s2)
        add     $t0, $t0, $t1
        sw      $t0, 0($s1)

        #delete from array
        la      $t0, metadata
        sub     $t0, $s1, $t0
        li      $t1, NODE_SIZE
        div     $a0, $t0, $t1
        jal     shift_left

        #update memory list size
        la      $t1, metadata_size
        lw      $t0, 0($t1)
        addi    $t0, $t0, -1
        sw      $t0, 0($t1)

        j       front_merge

front_merge:
        #can't front merge if it's first block
        la      $t0, metadata
        beq     $t0, $s1, free_done

        #check if previous chunk is free
        li      $t0, NODE_SIZE
        sub     $s2, $s1, $t0           #s2 is previous chunk
        lw      $t0, 4($s2)
        bne     $t0, $zero, free_done

        #we can front merge
        #merge sizes
        lw      $t0, 0($s1)
        lw      $t1, 0($s2)
        add     $t0, $t0, $t1
        sw      $t0, 0($s2)

        #delete from array
        la      $t0, metadata
        sub     $t0, $s2, $t0
        li      $t1, NODE_SIZE
        div     $a0, $t0, $t1
        jal     shift_left

        #update memory list size
        la      $t1, metadata_size
        lw      $t0, 0($t1)
        addi    $t0, $t0, -1
        sw      $t0, 0($t1)

        j       free_done

bad_free:
        li      $v0, PRINT_STRING
        la      $a0, bad_free_msg
        syscall

        li      $v0, EXIT
        syscall

free_done:
        lw      $s4, 0($sp)
        lw      $s3, 4($sp)
        lw      $s2, 8($sp)
        lw      $s1, 12($sp)
        lw      $s0, 16($sp)
        lw      $ra, 20($sp)
        addi    $sp, $sp, 24
        jr      $ra


#a0 - memory size
#v0 - allocated memory
malloc:
        addi    $sp, $sp, -32
        sw      $s6, 28($sp)
        sw      $s5, 24($sp)
        sw      $ra, 20($sp)
        sw      $s0, 16($sp)
        sw      $s1, 12($sp)
        sw      $s2, 8($sp)
        sw      $s3, 4($sp)
        sw      $s4, 0($sp)

        move    $s0, $a0                #s0 is memory size
        
        li      $t0, 4
        rem     $t0, $s0, $t0
        bne     $t0, $zero, next_4_mult

best_fit:
        li      $s1, 0                  #s1 is current
        li      $s2, -1                 #s2 is smallest delta
        li      $s3, -1                 #s3 is best fit
        la      $s4, metadata_size
        lw      $s4, 0($s4)             #s4 is metadata size
        
bf_loop:
        slt     $t0, $s1, $s4
        beq     $t0, $zero, bf_loop_done

        #get metadata
        li      $t0, NODE_SIZE
        mul     $t0, $t0, $s1
        la      $t1, metadata
        add     $s5, $t1, $t0           #s5 is the current metadata
        
        #if this chunk is allocated, continue
        lw      $t0, 4($s5)
        bne     $t0, $zero, bf_loop_end

        #if this chunk is too small, continue
        lw      $t0, 0($s5)
        slt     $t1, $t0, $s0
        bne     $t1, $zero, bf_loop_end

        #find delta
        lw      $a0, 0($s5)
        move    $a1, $s0
        jal     delta
        move    $s6, $v0                #s6 is current delta

        #use this chunk if we don't have anything yet
        li      $t0, -1
        beq     $s2, $t0, better_fit

        #use this chunk if the delta is smaller
        slt     $t1, $s6, $s2
        bne     $t1, $zero, better_fit

        j       bf_loop_end

better_fit:
        move    $s2, $s6
        move    $s3, $s5
        j       bf_loop_end

bf_loop_end:
        addi    $s1, $s1, 1
        j       bf_loop

bf_loop_done:
        j       best_fit_done

next_4_mult:
        li      $t0, 4
        rem     $t1, $s0, $t0
        sub     $t0, $t0, $t1
        add     $s0, $s0, $t0

        j       best_fit

best_fit_done:
        #check if no memory found
        li      $t0, -1
        beq     $t0, $s3, no_memory

        #split memory if needed
        lw      $t0, 0($s3)
        slt     $t0, $s0, $t0
        bne     $t0, $zero, split_mem
        
        j       malloc_done

split_mem:
        #make room for new entry
        la      $t0, metadata
        sub     $t0, $s3, $t0
        li      $t1, NODE_SIZE
        div     $a0, $t0, $t1
        jal     shift_right

        #increment memory list size
        la      $t1, metadata_size
        lw      $t0, 0($t1)
        addi    $t0, $t0, 1
        sw      $t0, 0($t1)

        #add new metadata chunk
        addi    $s5, $s3, NODE_SIZE             #s5 has the new node

        #mark new node as free
        sw      $zero, 4($s5)

        #mark size as delta
        sw      $s2, 0($s5)

        #add address pointer for new node
        lw      $t0, 8($s3)
        add     $t0, $t0, $s0
        sw      $t0, 8($s5)

        #update old mem chunk size
        sw      $s0, 0($s3)

        j       malloc_done

malloc_done:
        #mark this as allocated
        li      $t0, 1
        sw      $t0, 4($s3)

        #return best fit address
        lw      $v0, 8($s3)

        #clean up
        lw      $s4, 0($sp)
        lw      $s3, 4($sp)
        lw      $s2, 8($sp)
        lw      $s1, 12($sp)
        lw      $s0, 16($sp)
        lw      $ra, 20($sp)
        lw      $s5, 24($sp)
        lw      $s6, 28($sp)
        addi    $sp, $sp, 32
        jr      $ra

no_memory:
        li      $v0, PRINT_STRING
        la      $a0, no_memory_msg
        syscall

        li      $v0, EXIT
        syscall

#a0 - heap size
init_allocator:
        addi    $sp, $sp, -24
        sw      $ra, 20($sp)
        sw      $s0, 16($sp)
        sw      $s1, 12($sp)
        sw      $s2, 8($sp)
        sw      $s3, 4($sp)
        sw      $s4, 0($sp)

        move    $s0, $a0                        #s0 is heap size

        #size check
        li      $t0, MIN_HEAP_SIZE
        slt     $t1, $s0, $t0
        bne     $t1, $zero, invalid_arena

        li      $t0, MAX_HEAP_SIZE
        slt     $t1, $t0, $s0
        bne     $t1, $zero, invalid_arena

        la      $s1, metadata                   #s1 is metadata

        sw      $s0, 0($s1)                     #size=heap_size
        sw      $zero, 4($s1)                   #allocated=0
        la      $t0, heap
        sw      $t0, 8($s1)                     #address=heap

        j       init_allocator_done

invalid_arena:
        li      $v0, PRINT_STRING
        la      $a0, invalid_arena_msg
        syscall

        li      $v0, EXIT
        syscall

init_allocator_done:
        lw      $s4, 0($sp)
        lw      $s3, 4($sp)
        lw      $s2, 8($sp)
        lw      $s1, 12($sp)
        lw      $s0, 16($sp)
        lw      $ra, 20($sp)
        addi    $sp, $sp, 24
        jr      $ra


#prints the memory list
print_memory:
        addi    $sp, $sp, -24
        sw      $ra, 20($sp)
        sw      $s0, 16($sp)
        sw      $s1, 12($sp)
        sw      $s2, 8($sp)
        sw      $s3, 4($sp)
        sw      $s4, 0($sp)

        li      $s0, 0                  #s0 is index
        la      $s1, metadata_size
        lw      $s1, 0($s1)             #s1 is upper bound

print_loop:
        slt     $t0, $s0, $s1
        beq     $t0, $zero, print_done

        la      $s2, metadata
        li      $t0, NODE_SIZE
        mul     $t0, $t0, $s0
        add     $s2, $s2, $t0           #s2 is the metadata

        li      $v0, PRINT_STRING
        la      $a0, size_msg
        syscall

        li      $v0, PRINT_INT
        lw      $a0, 0($s2)
        syscall

        li      $v0, PRINT_STRING
        la      $a0, allocated_msg
        syscall

        li      $v0, PRINT_INT
        lw      $a0, 4($s2)
        syscall

        li      $v0, PRINT_STRING
        la      $a0, address_msg
        syscall

        #calculate relative addr
        lw      $t0, 8($s2)
        la      $t1, heap
        sub     $t0, $t0, $t1

        li      $v0, PRINT_INT
        move    $a0, $t0
        syscall

        li      $v0, PRINT_STRING
        la      $a0, end_bracket
        syscall

        addi    $s0, $s0, 1
        j       print_loop

print_done:
        lw      $s4, 0($sp)
        lw      $s3, 4($sp)
        lw      $s2, 8($sp)
        lw      $s1, 12($sp)
        lw      $s0, 16($sp)
        lw      $ra, 20($sp)
        addi    $sp, $sp, 24
        jr      $ra


#a0 - shift everything to the left of a0 (exclusive) by 1 to the left
shift_left:
        addi    $sp, $sp, -24
        sw      $ra, 20($sp)
        sw      $s0, 16($sp)
        sw      $s1, 12($sp)
        sw      $s2, 8($sp)
        sw      $s3, 4($sp)
        sw      $s4, 0($sp)

        addi    $s0, $a0, 1                     #s0 index
        la      $s1, metadata_size
        lw      $s1, 0($s1)                     #s1 is upper bound
        addi    $s1, $s1, -1

shift_left_loop:
        slt     $t0, $s0, $s1
        beq     $t0, $zero, shift_left_done

        addi    $s2, $s0, 1                     #s2 is index+1
        
        #find element at index + 1
        li      $t0, NODE_SIZE
        mul     $t0, $t0, $s2
        la      $t1, metadata
        add     $t0, $t0, $t1
        move    $s3, $t0                        #s3 is node at i + 1

        li      $t0, NODE_SIZE
        sub     $s4, $s3, $t0                   #s4 is node at i

        #shift left
        lw      $t0, 0($s3)
        sw      $t0, 0($s4)
        lw      $t0, 4($s3)
        sw      $t0, 4($s4)
        lw      $t0, 8($s3)
        sw      $t0, 8($s4)

        addi    $s0, $s0, 1
        j       shift_left_loop

shift_left_done:
        lw      $s4, 0($sp)
        lw      $s3, 4($sp)
        lw      $s2, 8($sp)
        lw      $s1, 12($sp)
        lw      $s0, 16($sp)
        lw      $ra, 20($sp)
        addi    $sp, $sp, 24
        jr      $ra


#a0 - shift everything to the right of a0 (exclusive) by 1 to the right
shift_right:
        addi    $sp, $sp, -24
        sw      $ra, 20($sp)
        sw      $s0, 16($sp)
        sw      $s1, 12($sp)
        sw      $s2, 8($sp)
        sw      $s3, 4($sp)
        sw      $s4, 0($sp)

        addi    $s0, $a0, 1                     #s0 is lower bound
        la      $s1, metadata_size
        lw      $s1, 0($s1)                     #s1 is index

shift_right_loop:
        slt     $t0, $s0, $s1
        beq     $t0, $zero, shift_right_done

        addi    $s2, $s1, -1                    #s2 is index-1
        
        #find element at index - 1
        li      $t0, NODE_SIZE
        mul     $t0, $t0, $s2
        la      $t1, metadata
        add     $t0, $t0, $t1
        move    $s3, $t0                        #s3 is node at i - 1

        addi    $s4, $s3, NODE_SIZE             #s4 is node at i

        #shift right
        lw      $t0, 0($s3)
        sw      $t0, 0($s4)
        lw      $t0, 4($s3)
        sw      $t0, 4($s4)
        lw      $t0, 8($s3)
        sw      $t0, 8($s4)

        addi    $s1, $s1, -1
        j       shift_right_loop

shift_right_done:
        lw      $s4, 0($sp)
        lw      $s3, 4($sp)
        lw      $s2, 8($sp)
        lw      $s1, 12($sp)
        lw      $s0, 16($sp)
        lw      $ra, 20($sp)
        addi    $sp, $sp, 24
        jr      $ra

#a0 - a
#a0 - b
#v0 - smaller of a and b
minimum:
        addi    $sp, $sp, -24
        sw      $ra, 20($sp)
        sw      $s0, 16($sp)
        sw      $s1, 12($sp)
        sw      $s2, 8($sp)
        sw      $s3, 4($sp)
        sw      $s4, 0($sp)

        slt     $t0, $a0, $a1
        bne     $t0, $zero, a_smaller

        move    $v0, $a1
        j       minimum_done

a_smaller:
        move    $v0, $a0
        j       minimum_done

minimum_done:
        lw      $s4, 0($sp)
        lw      $s3, 4($sp)
        lw      $s2, 8($sp)
        lw      $s1, 12($sp)
        lw      $s0, 16($sp)
        lw      $ra, 20($sp)
        addi    $sp, $sp, 24
        jr      $ra

#a0 - a
#a1 - b
#v0 - abs(a-b)
delta:
        addi    $sp, $sp, -24
        sw      $ra, 20($sp)
        sw      $s0, 16($sp)
        sw      $s1, 12($sp)
        sw      $s2, 8($sp)
        sw      $s3, 4($sp)
        sw      $s4, 0($sp)

        sub     $s0, $a0, $a1
        slt     $t0, $s0, $zero
        bne     $t0, $zero, negate_delta

        move    $v0, $s0
        j       delta_done

negate_delta:
        li      $t0, -1
        mul     $v0, $s0, $t0
        j       delta_done
        
delta_done:
        lw      $s4, 0($sp)
        lw      $s3, 4($sp)
        lw      $s2, 8($sp)
        lw      $s1, 12($sp)
        lw      $s0, 16($sp)
        lw      $ra, 20($sp)
        addi    $sp, $sp, 24
        jr      $ra

read:
        addi    $sp, $sp, -24
        sw      $ra, 20($sp)
        sw      $s0, 16($sp)
        sw      $s1, 12($sp)
        sw      $s2, 8($sp)
        sw      $s3, 4($sp)
        sw      $s4, 0($sp)
        
        #first line is allocator size
        li      $v0, READ_STRING
        la      $a0, buffer
        syscall
        jal     handle_init
read_loop:
        #read line
        la      $t0, buffer
        sb      $zero, 0($t0)           #clear first char so we know when we hit EOF
        li      $v0, READ_STRING
        la      $a0, buffer
        li      $a1, 128
        syscall

        #check if we finished
        la      $t0, buffer
        lb      $t1, 0($t0)
        li      $t2, '\0'
        beq     $t1, $t2, read_done

        #check for options
        #malloc case
        la      $t0, buffer
        lb      $t1, 0($t0)
        li      $t2, 'm'
        beq     $t1, $t2, handle_malloc
        
        #free case
        la      $t0, buffer
        lb      $t1, 0($t0)
        li      $t2, 'f'
        beq     $t1, $t2, handle_free

        #print case
        la      $t0, buffer
        lb      $t1, 0($t0)
        li      $t2, 'p'
        beq     $t1, $t2, handle_print

        #get case
        la      $t0, buffer
        lb      $t1, 0($t0)
        li      $t2, 'g'
        beq     $t1, $t2, handle_get

        #set case
        la      $t0, buffer
        lb      $t1, 0($t0)
        li      $t2, 's'
        beq     $t1, $t2, handle_set

        j       read_loop

handle_malloc:
        jal     parse_malloc
        j       read_loop

parse_malloc:
        addi    $sp, $sp, -24
        sw      $ra, 20($sp)
        sw      $s0, 16($sp)
        sw      $s1, 12($sp)
        sw      $s2, 8($sp)
        sw      $s3, 4($sp)
        sw      $s4, 0($sp)

        la      $s0, buffer
        lb      $s1, 2($s0)             #s1 contains the variable name

        addi    $s0, $s0, 4
        move    $a0, $s0
        jal     atoi
        move    $s2, $v0                #s2 contains the memory size

        move    $a0, $s2
        jal     malloc
        move    $s4, $v0                #s4 is allocated memory
        
        li      $v0, PRINT_CHAR
        move    $a0, $s1
        syscall

        li      $v0, PRINT_STRING
        la      $a0, malloc_message_middle
        syscall

        li      $v0, PRINT_INT
        move    $a0, $s2
        syscall

        li      $v0, PRINT_STRING
        la      $a0, right_parentheses
        syscall

        li      $v0, PRINT_STRING
        la      $a0, newline
        syscall

        #store this memory
        li      $t0, 'a'
        sub     $s3, $s1, $t0           #s3 is index

        li      $t0, 4
        mul     $t0, $t0, $s3
        la      $t1, variables
        add     $t0, $t0, $t1
        
        sw      $s4, 0($t0)

        lw      $s4, 0($sp)
        lw      $s3, 4($sp)
        lw      $s2, 8($sp)
        lw      $s1, 12($sp)
        lw      $s0, 16($sp)
        lw      $ra, 20($sp)
        addi    $sp, $sp, 24
        jr      $ra

handle_free:
        jal     parse_free
        j       read_loop

parse_free:
        addi    $sp, $sp, -24
        sw      $ra, 20($sp)
        sw      $s0, 16($sp)
        sw      $s1, 12($sp)
        sw      $s2, 8($sp)
        sw      $s3, 4($sp)
        sw      $s4, 0($sp)

        la      $s0, buffer
        lb      $s1, 2($s0)             #s1 contains the variable name

        li      $t0, 'a'
        sub     $t0, $s1, $t0
        li      $t1, 4
        mul     $t1, $t1, $t0
        la      $t0, variables
        add     $t0, $t0, $t1

        lw      $a0, 0($t0)
        jal     free


        li      $v0, PRINT_STRING
        la      $a0, free_message_front
        syscall

        li      $v0, PRINT_CHAR
        move    $a0, $s1
        syscall

        li      $v0, PRINT_STRING
        la      $a0, right_parentheses
        syscall

        li      $v0, PRINT_STRING
        la      $a0, newline
        syscall

        lw      $s4, 0($sp)
        lw      $s3, 4($sp)
        lw      $s2, 8($sp)
        lw      $s1, 12($sp)
        lw      $s0, 16($sp)
        lw      $ra, 20($sp)
        addi    $sp, $sp, 24
        jr      $ra

handle_print:
        jal     parse_print
        j       read_loop

parse_print:
        addi    $sp, $sp, -24
        sw      $ra, 20($sp)
        sw      $s0, 16($sp)
        sw      $s1, 12($sp)
        sw      $s2, 8($sp)
        sw      $s3, 4($sp)
        sw      $s4, 0($sp)

        jal     print_memory

        lw      $s4, 0($sp)
        lw      $s3, 4($sp)
        lw      $s2, 8($sp)
        lw      $s1, 12($sp)
        lw      $s0, 16($sp)
        lw      $ra, 20($sp)
        addi    $sp, $sp, 24
        jr      $ra

handle_get:
        jal     parse_get
        j       read_loop

parse_get:
        addi    $sp, $sp, -24
        sw      $ra, 20($sp)
        sw      $s0, 16($sp)
        sw      $s1, 12($sp)
        sw      $s2, 8($sp)
        sw      $s3, 4($sp)
        sw      $s4, 0($sp)

        la      $s0, buffer
        lb      $s1, 2($s0)             #s1 contains the variable name

        li      $t0, 'a'
        sub     $t0, $s1, $t0
        li      $t1, 4
        mul     $t1, $t1, $t0
        la      $t0, variables
        add     $t0, $t0, $t1

        lw      $s2, 0($t0)             #s2 contains the variable value

        li      $v0, PRINT_CHAR
        move    $a0, $s1
        syscall

        li      $v0, PRINT_STRING
        la      $a0, double_equals
        syscall

        li      $v0, PRINT_INT
        move    $a0, $s2
        syscall

        li      $v0, PRINT_STRING
        la      $a0, newline
        syscall

        lw      $s4, 0($sp)
        lw      $s3, 4($sp)
        lw      $s2, 8($sp)
        lw      $s1, 12($sp)
        lw      $s0, 16($sp)
        lw      $ra, 20($sp)
        addi    $sp, $sp, 24
        jr      $ra

handle_set:
        jal     parse_set
        j       read_loop

parse_set:
        addi    $sp, $sp, -24
        sw      $ra, 20($sp)
        sw      $s0, 16($sp)
        sw      $s1, 12($sp)
        sw      $s2, 8($sp)
        sw      $s3, 4($sp)
        sw      $s4, 0($sp)

        la      $s0, buffer
        lb      $s1, 2($s0)             #s1 has the variable name

        addi    $s0, $s0, 4
        move    $a0, $s0
        jal     atoi
        move    $s2, $v0                #s2 has the variable value

        li      $v0, PRINT_CHAR
        move    $a0, $s1
        syscall

        li      $v0, PRINT_STRING
        la      $a0, equals
        syscall

        li      $v0, PRINT_INT
        move    $a0, $s2
        syscall

        li      $v0, PRINT_STRING
        la      $a0, newline
        syscall

        li      $t0, 'a'
        sub     $t0, $s1, $t0
        li      $t1, 4
        mul     $t1, $t1, $t0
        la      $t0, variables
        add     $t0, $t0, $t1

        sw      $s2, 0($t0)

        lw      $s4, 0($sp)
        lw      $s3, 4($sp)
        lw      $s2, 8($sp)
        lw      $s1, 12($sp)
        lw      $s0, 16($sp)
        lw      $ra, 20($sp)
        addi    $sp, $sp, 24
        jr      $ra

handle_init:
        jal     parse_init
        j       read_loop

parse_init:
        addi    $sp, $sp, -24
        sw      $ra, 20($sp)
        sw      $s0, 16($sp)
        sw      $s1, 12($sp)
        sw      $s2, 8($sp)
        sw      $s3, 4($sp)
        sw      $s4, 0($sp)

        #convert arena size to integer
        la      $a0, buffer
        jal     atoi
        move    $s0, $v0        #s0 contains arena size integer

        move    $a0, $s0
        jal     init_allocator

        #print initialization msg
        li      $v0, PRINT_STRING
        la      $a0, init_message_front
        syscall
        li      $v0, PRINT_INT
        move    $a0, $s0
        syscall
        li      $v0, PRINT_STRING
        la      $a0, init_message_back
        syscall

        lw      $s4, 0($sp)
        lw      $s3, 4($sp)
        lw      $s2, 8($sp)
        lw      $s1, 12($sp)
        lw      $s0, 16($sp)
        lw      $ra, 20($sp)
        addi    $sp, $sp, 24
        jr      $ra

#a0 - the exponent
#a1 - the base
#v0 - base raised to the exponent
exponent:
        addi    $sp, $sp, -24
        sw      $ra, 20($sp)
        sw      $s0, 16($sp)
        sw      $s1, 12($sp)
        sw      $s2, 8($sp)
        sw      $s3, 4($sp)
        sw      $s4, 0($sp)

        li      $s0, 1                  #running sum
        li      $s1, 0                  #index
exponent_loop:
        beq     $s1, $a0, exponent_done
        mul     $s0, $s0, $a1
        addi    $s1, $s1, 1
        j       exponent_loop
exponent_done:
        move    $v0, $s0

        lw      $s4, 0($sp)
        lw      $s3, 4($sp)
        lw      $s2, 8($sp)
        lw      $s1, 12($sp)
        lw      $s0, 16($sp)
        lw      $ra, 20($sp)
        addi    $sp, $sp, 24
        jr      $ra

#a0 - start of a null terminated digit string
#v0 - the integer value of that digit string
atoi:
        addi    $sp, $sp, -28
        sw      $s5, 24($sp)
        sw      $ra, 20($sp)
        sw      $s0, 16($sp)
        sw      $s1, 12($sp)
        sw      $s2, 8($sp)
        sw      $s3, 4($sp)
        sw      $s4, 0($sp)

        li      $s0, 0          #counter
        move    $s1, $a0
next_char:
        lb      $t1, 0($s1)
        beq     $t1, $zero, end_char
        addi    $s1, $s1, 1
        addi    $s0, $s0, 1
        j       next_char
end_char:
        addi    $s0, $s0, -1    #we counted null terminator
        move    $s2, $s0        #number of digits stored in both s2 and s0

        addi    $s0, $s0, -1    #off by one err with this loop approach
        li      $s3, 0          #current digit place
        li      $s5, 0
sum_loop:
        li      $t0, -1
        beq     $s0, $t0, sum_done

        #get digit
        add     $s1, $a0, $s0
        lb      $s1, 0($s1)
        li      $t0, '0'
        sub     $s1, $s1, $t0   #s1 contains integer value of the single digit

        #convert digit to integer
        move    $s4, $a0
        move    $a0, $s3
        li      $a1, 10
        jal     exponent
        move    $a0, $s4

        #add to running sum
        mul     $t0, $v0, $s1   #t0 contains digit value multiplied by magnitude
        add     $s5, $s5, $t0
        addi    $s0, $s0, -1
        addi    $s3, $s3, 1
        j       sum_loop
sum_done:
        move    $v0, $s5

        lw      $s4, 0($sp)
        lw      $s3, 4($sp)
        lw      $s2, 8($sp)
        lw      $s1, 12($sp)
        lw      $s0, 16($sp)
        lw      $ra, 20($sp)
        lw      $s5, 24($sp)
        addi    $sp, $sp, 28
        jr      $ra


read_done:
        lw      $s4, 0($sp)
        lw      $s3, 4($sp)
        lw      $s2, 8($sp)
        lw      $s1, 12($sp)
        lw      $s0, 16($sp)
        lw      $ra, 20($sp)
        addi    $sp, $sp, 24
        jr      $ra

done:
        lw      $s4, 0($sp)
        lw      $s3, 4($sp)
        lw      $s2, 8($sp)
        lw      $s1, 12($sp)
        lw      $s0, 16($sp)
        lw      $ra, 20($sp)
        addi    $sp, $sp, 24
        jr      $ra

