      case PRIMITIVE1 :
        switch (instr & 0x0F) {
          case 0 :
            TRACE("  (%s <%d>)\n", "#%halt", 0);
            return;

          case 1 :
            TRACE("  (%s <%d>)\n", "return", 1);
            reg1 = pop();
            primitive_return();
            env = new_pair(reg1, env);
            break;

          case 2 :
            TRACE("  (%s <%d>)\n", "pop", 0);
            primitive_pop();
            break;

          case 3 :
            TRACE("  (%s <%d>)\n", "get-cont", 0);
            primitive_get_cont();
            env = new_pair(reg1, env);
            break;

          case 4 :
            TRACE("  (%s <%d>)\n", "graft-to-cont", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_graft_to_cont();
            break;

          case 5 :
            TRACE("  (%s <%d>)\n", "return-to-cont", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_return_to_cont();
            env = new_pair(reg1, env);
            break;

          case 6 :
            TRACE("  (%s <%d>)\n", "pair?", 1);
            reg1 = pop();
            primitive_pair_p();
            env = new_pair(reg1, env);
            break;

          case 7 :
            TRACE("  (%s <%d>)\n", "cons", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_cons();
            env = new_pair(reg1, env);
            break;

          case 8 :
            TRACE("  (%s <%d>)\n", "car", 1);
            reg1 = pop();
            primitive_car();
            env = new_pair(reg1, env);
            break;

          case 9 :
            TRACE("  (%s <%d>)\n", "cdr", 1);
            reg1 = pop();
            primitive_cdr();
            env = new_pair(reg1, env);
            break;

          case 10 :
            TRACE("  (%s <%d>)\n", "set-car!", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_set_car_bang();
            break;

          case 11 :
            TRACE("  (%s <%d>)\n", "set-cdr!", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_set_cdr_bang();
            break;

          case 12 :
            TRACE("  (%s <%d>)\n", "null?", 1);
            reg1 = pop();
            primitive_null_p();
            env = new_pair(reg1, env);
            break;

          case 13 :
            TRACE("  (%s <%d>)\n", "number?", 1);
            reg1 = pop();
            primitive_number_p();
            env = new_pair(reg1, env);
            break;

          case 14 :
            TRACE("  (%s <%d>)\n", "=", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_equal();
            env = new_pair(reg1, env);
            break;

          case 15 :
            TRACE("  (%s <%d>)\n", "#%+", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_add();
            env = new_pair(reg1, env);
            break;
        }
        break;

      case PRIMITIVE2 :
        switch (instr & 0x0F) {
          case 0 :
            TRACE("  (%s <%d>)\n", "#%-", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_sub();
            env = new_pair(reg1, env);
            break;

          case 1 :
            TRACE("  (%s <%d>)\n", "#%mul-non-neg", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_mul_non_neg();
            env = new_pair(reg1, env);
            break;

          case 2 :
            TRACE("  (%s <%d>)\n", "#%div-non-neg", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_div_non_neg();
            env = new_pair(reg1, env);
            break;

          case 3 :
            TRACE("  (%s <%d>)\n", "#%rem-non-neg", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_rem_non_neg();
            env = new_pair(reg1, env);
            break;

          case 4 :
            TRACE("  (%s <%d>)\n", "<", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_lt();
            env = new_pair(reg1, env);
            break;

          case 5 :
            TRACE("  (%s <%d>)\n", ">", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_gt();
            env = new_pair(reg1, env);
            break;

          case 6 :
            TRACE("  (%s <%d>)\n", "bitwise-ior", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_bitwise_ior();
            env = new_pair(reg1, env);
            break;

          case 7 :
            TRACE("  (%s <%d>)\n", "bitwise-xor", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_bitwise_xor();
            env = new_pair(reg1, env);
            break;

          case 8 :
            TRACE("  (%s <%d>)\n", "bitwise-and", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_bitwise_and();
            env = new_pair(reg1, env);
            break;

          case 9 :
            TRACE("  (%s <%d>)\n", "bitwise-not", 1);
            reg1 = pop();
            primitive_bitwise_not();
            env = new_pair(reg1, env);
            break;

          case 10 :
            TRACE("  (%s <%d>)\n", "eq?", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_eq_p();
            env = new_pair(reg1, env);
            break;

          case 11 :
            TRACE("  (%s <%d>)\n", "not", 1);
            reg1 = pop();
            primitive_not();
            env = new_pair(reg1, env);
            break;

          case 12 :
            TRACE("  (%s <%d>)\n", "symbol?", 1);
            reg1 = pop();
            primitive_symbol_p();
            env = new_pair(reg1, env);
            break;

          case 13 :
            TRACE("  (%s <%d>)\n", "boolean?", 1);
            reg1 = pop();
            primitive_boolean_p();
            env = new_pair(reg1, env);
            break;

          case 14 :
            TRACE("  (%s <%d>)\n", "string?", 1);
            reg1 = pop();
            primitive_string_p();
            env = new_pair(reg1, env);
            break;

          case 15 :
            TRACE("  (%s <%d>)\n", "string->list", 1);
            reg1 = pop();
            primitive_string2list();
            env = new_pair(reg1, env);
            break;
        }
        break;

      case PRIMITIVE3 :
        switch (instr & 0x0F) {
          case 0 :
            TRACE("  (%s <%d>)\n", "list->string", 1);
            reg1 = pop();
            primitive_list2string();
            env = new_pair(reg1, env);
            break;

          case 1 :
            TRACE("  (%s <%d>)\n", "u8vector?", 1);
            reg1 = pop();
            primitive_u8vector_p();
            env = new_pair(reg1, env);
            break;

          case 2 :
            TRACE("  (%s <%d>)\n", "#%make-u8vector", 1);
            reg1 = pop();
            primitive_make_u8vector();
            env = new_pair(reg1, env);
            break;

          case 3 :
            TRACE("  (%s <%d>)\n", "u8vector-ref", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_u8vector_ref();
            env = new_pair(reg1, env);
            break;

          case 4 :
            TRACE("  (%s <%d>)\n", "u8vector-set!", 3);
            reg3 = pop();
            reg2 = pop();
            reg1 = pop();
            primitive_u8vector_set();
            break;

          case 5 :
            TRACE("  (%s <%d>)\n", "u8vector-length", 1);
            reg1 = pop();
            primitive_u8vector_length();
            env = new_pair(reg1, env);
            break;

          case 6 :
            TRACE("  (%s <%d>)\n", "print", 1);
            reg1 = pop();
            primitive_print();
            break;

          case 7 :
            TRACE("  (%s <%d>)\n", "clock", 0);
            primitive_clock();
            env = new_pair(reg1, env);
            break;

          case 8 :
            TRACE("  (%s <%d>)\n", "#%getchar-wait", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_getchar_wait();
            env = new_pair(reg1, env);
            break;

          case 9 :
            TRACE("  (%s <%d>)\n", "#%putchar", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_putchar();
            break;

          case 10 :
            TRACE("  (%s <%d>)\n", "GPIO", 2);
            reg2 = pop();
            reg1 = pop();
            primitive_gpio();
            env = new_pair(reg1, env);
            break;
        }
        break;

