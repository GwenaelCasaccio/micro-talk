#pragma once
#include "stack_vm.hpp"
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class Disassembler {
  public:
    static std::string opcode_to_string(Opcode op) {
        switch (op) {
            case Opcode::HALT:
                return "HALT";
            case Opcode::PUSH:
                return "PUSH";
            case Opcode::POP:
                return "POP";
            case Opcode::DUP:
                return "DUP";
            case Opcode::ADD:
                return "ADD";
            case Opcode::SUB:
                return "SUB";
            case Opcode::MUL:
                return "MUL";
            case Opcode::DIV:
                return "DIV";
            case Opcode::MOD:
                return "MOD";
            case Opcode::EQ:
                return "EQ";
            case Opcode::LT:
                return "LT";
            case Opcode::GT:
                return "GT";
            case Opcode::JMP:
                return "JMP";
            case Opcode::JZ:
                return "JZ";
            case Opcode::ENTER:
                return "ENTER";
            case Opcode::LEAVE:
                return "LEAVE";
            case Opcode::CALL:
                return "CALL";
            case Opcode::RET:
                return "RET";
            case Opcode::IRET:
                return "IRET";
            case Opcode::LOAD:
                return "LOAD";
            case Opcode::STORE:
                return "STORE";
            case Opcode::BP_LOAD:
                return "BP_LOAD";
            case Opcode::BP_STORE:
                return "BP_STORE";
            case Opcode::PRINT:
                return "PRINT";
            case Opcode::PRINT_STR:
                return "PRINT_STR";
            case Opcode::AND:
                return "AND";
            case Opcode::OR:
                return "OR";
            case Opcode::XOR:
                return "XOR";
            case Opcode::SHL:
                return "SHL";
            case Opcode::SHR:
                return "SHR";
            case Opcode::ASHR:
                return "ASHR";
            case Opcode::CLI:
                return "CLI";
            case Opcode::STI:
                return "STI";
            case Opcode::SIGNAL_REG:
                return "SIGNAL_REG";
            default:
                return "UNKNOWN";
        }
    }

    static bool has_operand(Opcode op) {
        switch (op) {
            case Opcode::PUSH:
            case Opcode::JMP:
            case Opcode::JZ:
                return true;
            default:
                return false;
        }
    }

    static int operand_count(Opcode op) {
        switch (op) {
            case Opcode::PUSH:
            case Opcode::JMP:
            case Opcode::JZ:
            case Opcode::ENTER:
            case Opcode::LEAVE:
            case Opcode::RET:
                return 1;
            case Opcode::CALL:
                return 2;
            default:
                return 0;
        }
    }

    static void disassemble(const std::vector<uint64_t>& bytecode, std::ostream& out = std::cout) {
        size_t ip = 0;
        while (ip < bytecode.size()) {
            // Print address
            out << std::setw(6) << std::setfill('0') << ip << ": ";

            // Get opcode
            const auto op = static_cast<Opcode>(bytecode[ip] & 0xFF);
            const std::string op_name = opcode_to_string(op);

            // Print instruction
            out << std::left << std::setw(12) << std::setfill(' ') << op_name;

            ip++;

            // Print operands if present
            int op_count = operand_count(op);
            for (int i = 0; i < op_count && ip < bytecode.size(); i++) {
                if (i > 0)
                    out << ", ";

                uint64_t operand = bytecode[ip];

                // For jump/call instructions, show as address
                if (op == Opcode::JMP || op == Opcode::JZ || op == Opcode::CALL) {
                    out << "@" << operand;
                } else {
                    // For PUSH and other operands, show value (decimal and hex)
                    out << operand;
                    if (operand > 9 || operand == 0) {
                        out << " (0x" << std::hex << operand << std::dec << ")";
                    }
                }

                ip++;
            }

            out << '\n';
        }
    }

    static std::string disassemble_to_string(const std::vector<uint64_t>& bytecode) {
        std::ostringstream oss;
        disassemble(bytecode, oss);
        return oss.str();
    }
};
