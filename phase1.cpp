#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <iomanip>
#include <bitset>
using namespace std;

// Define opcode, funct3, and funct7 mappings for RISC-V instructions
unordered_map<string, string> opcodeMap = {
    {"add", "0110011"}, {"sub", "0110011"}, {"and", "0110011"}, {"or", "0110011"},
    {"sll", "0110011"}, {"slt", "0110011"}, {"sra", "0110011"}, {"srl", "0110011"},
    {"xor", "0110011"}, {"mul", "0110011"}, {"div", "0110011"}, {"rem", "0110011"},
    {"addi", "0010011"}, {"andi", "0010011"}, {"ori", "0010011"}, {"jalr", "1100111"},
    {"lb", "0000011"}, {"ld", "0000011"}, {"lh", "0000011"}, {"lw", "0000011"},
    {"sb", "0100011"}, {"sw", "0100011"}, {"sd", "0100011"}, {"sh", "0100011"},
    {"beq", "1100011"}, {"bne", "1100011"}, {"bge", "1100011"}, {"blt", "1100011"},
    {"auipc", "0010111"}, {"lui", "0110111"}, {"jal", "1101111"}
};

unordered_map<string, string> funct3Map = {
    {"add", "000"}, {"sub", "000"}, {"and", "111"}, {"or", "110"},
    {"sll", "001"}, {"slt", "010"}, {"sra", "101"}, {"srl", "101"},
    {"xor", "100"}, {"mul", "000"}, {"div", "100"}, {"rem", "110"},
    {"addi", "000"}, {"andi", "111"}, {"ori", "110"}, {"jalr", "000"},
    {"lb", "000"}, {"ld", "011"}, {"lh", "001"}, {"lw", "010"},
    {"sb", "000"}, {"sw", "010"}, {"sd", "011"}, {"sh", "001"},
    {"beq", "000"}, {"bne", "001"}, {"bge", "101"}, {"blt", "100"}
};

unordered_map<string, string> funct7Map = {
    {"add", "0000000"}, {"sub", "0100000"}, {"sra", "0100000"}, {"srl", "0000000"},
    {"mul", "0000001"}, {"div", "0000001"}, {"rem", "0000001"}
};

string registerToBinary(string reg) {
    int regNum = stoi(reg.substr(1));
    return bitset<5>(regNum).to_string();
}

string parseRFormat(string inst, string rd, string rs1, string rs2) {
    return funct7Map[inst] + registerToBinary(rs2) + registerToBinary(rs1) + 
           funct3Map[inst] + registerToBinary(rd) + opcodeMap[inst];
}

string parseIFormat(string inst, string rd, string rs1, int imm) {
    return bitset<12>(imm).to_string() + registerToBinary(rs1) + funct3Map[inst] +
           registerToBinary(rd) + opcodeMap[inst];
}

string formatBinaryInstruction(string opcode, string funct3, string funct7, string rd, string rs1, string rs2, string imm) {
    stringstream ss;

    // Print opcode, funct3, funct7, and register fields in binary format
    ss << opcode << "-" << funct3 << "-";

    // For R-format instructions (funct7 is present)
    if (!funct7.empty()) {
        ss << funct7 << "-";
    } else {
        ss << "NULL-";
    }

    // Register and immediate values
    ss << rd << "-" << rs1 << "-";

    if (!rs2.empty()) {
        ss << rs2 << "-";
    }

    if (!imm.empty()) {
        ss << imm;
    } else {
        ss << "NULL";
    }

    return ss.str();
}

void assemble(string inputFile, string outputFile) {
    ifstream inFile(inputFile);
    ofstream outFile(outputFile);
    string line;
    int address = 0;
    
    while (getline(inFile, line)) {
        istringstream iss(line);
        string inst, rd, rs1, rs2;
        string formatedInstruction;
        int imm;
        iss >> inst;

        if (opcodeMap.find(inst) != opcodeMap.end()) {
            if (funct3Map.find(inst) != funct3Map.end()) {
                if (funct7Map.find(inst) != funct7Map.end()) {
                    iss >> rd >> rs1 >> rs2;
                    bitset<32> machineCode(parseRFormat(inst, rd, rs1, rs2)); 
                    formatedInstruction = formatBinaryInstruction(opcodeMap[inst], funct3Map[inst], funct7Map[inst], registerToBinary(rd), registerToBinary(rs1), registerToBinary(rs2), "");
                    outFile << "0x" << hex << address << " 0x" 
                            << setw(8) << setfill('0') << stoul(machineCode.to_string(), nullptr, 2)
                            << " , " << line << " # " << formatedInstruction << endl;
                } else {
                    iss >> rd >> rs1 >> imm;
                    bitset<32> machineCode(parseIFormat(inst, rd, rs1, imm));
                    formatedInstruction = formatBinaryInstruction(opcodeMap[inst], funct3Map[inst], "", registerToBinary(rd), registerToBinary(rs1), "", bitset<12>(imm).to_string());
                    outFile << "0x" << hex << address << " 0x" << setw(8) << setfill('0') << stoul(machineCode.to_string(), nullptr, 2) << " , " << line
                    << " # " << formatedInstruction << endl;
                }
            }
        }
        address += 4;
    }
    inFile.close();
    outFile.close();
}

int main() {
    assemble("input.asm", "output.mc");
    cout << "Assembly translation complete. Check output.mc" << endl;
    return 0;
}