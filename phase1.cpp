#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <iomanip>
#include <stdexcept>
using namespace std;

// Define opcode, funct3, and funct7 mappings for RISC-V instructions
unordered_map<string, string> opcodeMap = {
    {"add", "0110011"}, {"and", "0110011"}, {"or", "0110011"}, {"sll", "0110011"}, {"slt", "0110011"}, {"sra", "0110011"}, {"srl", "0110011"}, {"sub", "0110011"}, {"xor", "0110011"}, {"mul", "0110011"}, {"div", "0110011"}, {"rem", "0110011"}, {"addi", "0010011"}, {"andi", "0010011"}, {"ori", "0010011"}, {"jalr", "1100111"}, {"lb", "0000011"}, {"ld", "0000011"}, {"lh", "0000011"}, {"lw", "0000011"}, {"sb", "0100011"}, {"sw", "0100011"}, {"sd", "0100011"}, {"sh", "0100011"}, {"beq", "1100011"}, {"bne", "1100011"}, {"bge", "1100011"}, {"blt", "1100011"}, {"auipc", "0010111"}, {"lui", "0110111"}, {"jal", "1101111"}};

unordered_map<string, string> funct3Map = {
    {"add", "000"}, {"and", "111"}, {"or", "110"}, {"sll", "001"}, {"slt", "010"}, {"sra", "101"}, {"srl", "101"}, {"sub", "000"}, {"xor", "100"}, {"mul", "000"}, {"div", "100"}, {"rem", "110"}, {"addi", "000"}, {"andi", "111"}, {"ori", "110"}, {"jalr", "000"}, {"lb", "000"}, {"ld", "011"}, {"lh", "001"}, {"lw", "010"}, {"sb", "000"}, {"sw", "010"}, {"sd", "011"}, {"sh", "001"}, {"beq", "000"}, {"bne", "001"}, {"bge", "101"}, {"blt", "100"}};

unordered_map<string, string> funct7Map = {
    {"add", "0000000"}, {"sub", "0100000"}, {"sra", "0100000"}, {"srl", "0000000"}, {"mul", "0000001"}, {"div", "0000001"}, {"rem", "0000001"}};

unordered_map<string, int> labelAddresses;    // Stores label names and their corresponding addresses
unordered_map<int, string> instructionLabels; // Stores instruction indices with labels

unordered_map<string, vector<int>> dataDirectives; // Stores addresses for data segments

// Function to convert register to 5-bit binary string
string registerToBinary(string reg)
{
    int regNum = stoi(reg.substr(1)); // Remove the 'x' prefix and convert to int
    return bitset<5>(regNum).to_string();
}

// Function to parse R-format instructions
string parseRFormat(string inst, string rd, string rs1, string rs2)
{
    return funct7Map[inst] + registerToBinary(rs2) + registerToBinary(rs1) + funct3Map[inst] +
           registerToBinary(rd) + opcodeMap[inst];
}

// Function to parse I-format instructions
string parseIFormat(string inst, string rd, string rs1, int imm)
{
    return bitset<12>(imm).to_string() + registerToBinary(rs1) + funct3Map[inst] +
           registerToBinary(rd) + opcodeMap[inst];
}

// Function to parse S-format instructions
string parseSFormat(string inst, string rs1, string rs2, int imm)
{
    return bitset<7>(imm & 0x7f).to_string() + registerToBinary(rs2) + registerToBinary(rs1) +
           funct3Map[inst] + bitset<5>((imm >> 7) & 0x1f).to_string() + opcodeMap[inst];
}

// Function to parse SB-format instructions
string parseSBFormat(string inst, string rs1, string rs2, int imm)
{
    return bitset<12>(imm).to_string() + registerToBinary(rs2) + registerToBinary(rs1) +
           funct3Map[inst] + bitset<7>((imm >> 7) & 0x7f).to_string() + opcodeMap[inst];
}

// Function to parse U-format instructions
string parseUFormat(string inst, string rd, int imm)
{
    return bitset<20>(imm).to_string() + registerToBinary(rd) + opcodeMap[inst];
}

// Function to parse UJ-format instructions
string parseUJFormat(string inst, string rd, int imm)
{
    return bitset<20>(imm).to_string() + registerToBinary(rd) + opcodeMap[inst];
}

void processDirectives(string line, int &address)
{
    // Handle .data section
    if (line.find(".data") != string::npos)
    {
        address = 0x10000000; // Data segment starts here
    }
    // Handle .text section
    else if (line.find(".text") != string::npos)
    {
        address = 0x0; // Text segment starts here
    }
    // Handle .word (4 bytes)
    else if (line.find(".word") != string::npos)
    {
        stringstream ss(line);
        string token;
        ss >> token; // Skip .word
        int value;
        // Process all values in the line (after the .word token)
        while (ss >> value) 
        {
            dataDirectives["word"].push_back(address);
            address += 4; // Increment address by 4 bytes for each word
        }
    }
    // Handle .byte (1 byte)
    else if (line.find(".byte") != string::npos)
    {
        stringstream ss(line);
        string token;
        ss >> token; // Skip .byte
        int value;
        // Process all values in the line (after the .byte token)
        while (ss >> value) 
        {
            dataDirectives["byte"].push_back(address);
            address += 1; // Increment address by 1 byte for each byte
        }
    }
    // Handle .half (2 bytes)
    else if (line.find(".half") != string::npos)
    {
        stringstream ss(line);
        string token;
        ss >> token; // Skip .half
        int value;
        // Process all values in the line (after the .half token)
        while (ss >> value) 
        {
            dataDirectives["half"].push_back(address);
            address += 2; // Increment address by 2 bytes for each half word
        }
    }
    // Handle .double (8 bytes)
    else if (line.find(".double") != string::npos)
    {
        stringstream ss(line);
        string token;
        ss >> token; // Skip .double
        long long value;
        // Process all values in the line (after the .double token)
        while (ss >> value) 
        {
            dataDirectives["double"].push_back(address);
            address += 8; // Increment address by 8 bytes for each double word
        }
    }
    // Handle .asciiz (null-terminated string)
    else if (line.find(".asciiz") != string::npos)
    {
        stringstream ss(line);
        string token;
        ss >> token; // Skip .asciiz
        string value;
        ss >> ws; // Skip any leading whitespace before the string value
        getline(ss, value); // Read the string (potentially with quotes)

        // Remove leading and trailing quotes if they exist
        if (value.size() > 1 && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.size() - 2);  // Remove quotes
        }

        // Add each character of the string to memory
        for (char c : value)
        {
            dataDirectives["asciiz"].push_back(address);
            address += 1;
        }
        dataDirectives["asciiz"].push_back(address); // Add null terminator
        address += 1;
    }
}

void firstPass(string inputFile)
{
    ifstream inFile(inputFile);
    string line;
    int address = 0;
    bool inTextSegment = false; // Keeps track of whether we're in the .text segment
    bool inDataSegment = false; // Keeps track of whether we're in the .data segment

    while (getline(inFile, line))
    {
        istringstream iss(line);
        string token;
        iss >> token;

        // Handle .text and .data directives
        if (token == ".text")
        {
            inTextSegment = true;
            inDataSegment = false;
            continue;
        }
        if (token == ".data")
        {
            inDataSegment = true;
            inTextSegment = false;
            address = 0x10000000; // Start of the data segment at 0x10000000
            continue;
        }

        // Handle labels
        if (token.back() == ':')
        {
            string label = token.substr(0, token.size() - 1);
            labelAddresses[label] = address;
            continue; // Skip the label and continue processing the next token
        }

        // Process instructions and data
        if (inTextSegment || inDataSegment)
        {
            // Handle data directives
            if (inDataSegment)
            {
                if (line.find(".byte") != string::npos)
                {
                    stringstream ss(line);
                    string directive;
                    ss >> directive; // Skip the .byte part
                    int value;
                    while (ss >> value) 
                    {
                        dataDirectives["byte"].push_back(address);
                        cout << "0x" << hex << address << " 0x" << value << " , .byte " << value << endl;
                        address += 1; // 1 byte for each value
                    }
                }
                else if (line.find(".half") != string::npos)
                {
                    stringstream ss(line);
                    string directive;
                    ss >> directive; // Skip the .half part
                    int value;
                    while (ss >> value) 
                    {
                        dataDirectives["half"].push_back(address);
                        cout << "0x" << hex << address << " 0x" << value << " , .half " << value << endl;
                        address += 2; // 2 bytes for each half
                    }
                }
                else if (line.find(".word") != string::npos)
                {
                    stringstream ss(line);
                    string directive;
                    ss >> directive; // Skip the .word part
                    int value;
                    while (ss >> value) 
                    {
                        dataDirectives["word"].push_back(address);
                        cout << "0x" << hex << address << " 0x" << value << " , .word " << value << endl;
                        address += 4; // 4 bytes for each word
                    }
                }
                else if (line.find(".double") != string::npos)
                {
                    stringstream ss(line);
                    string directive;
                    ss >> directive; // Skip the .double part
                    long long value;
                    while (ss >> value) 
                    {
                        dataDirectives["double"].push_back(address);
                        cout << "0x" << hex << address << " 0x" << value << " , .double " << value << endl;
                        address += 8; // 8 bytes for each double
                    }
                }
                else if (line.find(".asciiz") != string::npos)
                {
                    stringstream ss(line);
                    string directive;
                    ss >> directive; // Skip the .asciiz part
                    string str;
                    ss >> ws; // Skip leading whitespace
                    getline(ss, str); // Read the string

                    // Remove leading and trailing quotes if they exist
                    if (str.size() > 1 && str.front() == '"' && str.back() == '"') {
                        str = str.substr(1, str.size() - 2); // Remove quotes
                    }

                    // Process each character in the string
                    for (char c : str)
                    {
                        dataDirectives["asciiz"].push_back(address);
                        cout << "0x" << hex << address << " 0x" << int(c) << " , .asciiz " << c << endl;
                        address += 1; // Each character is 1 byte
                    }
                    dataDirectives["asciiz"].push_back(address); // Null terminator
                    cout << "0x" << hex << address << " 0x0 , .asciiz \\0" << endl;
                    address += 1;
                }
            }
            else if (inTextSegment)
            {
                address += 4; // Each instruction is 4 bytes
            }
        }
    }
    inFile.close();
}

void secondPass(string inputFile, string outputFile)
{
    ifstream inFile(inputFile);
    ofstream outFile(outputFile);
    string line;
    int address = 0;
    bool inTextSegment = false; // Keeps track of whether we're in the .text segment
    bool inDataSegment = false; // Keeps track of whether we're in the .data segment

    while (getline(inFile, line))
    {
        istringstream iss(line);
        string token;
        iss >> token;

        // Skip empty lines or comments
        if (line.empty() || line[0] == '#')
            continue;

        // Handle .text and .data directives
        if (token == ".text")
        {
            inTextSegment = true;
            inDataSegment = false;
            continue;
        }
        if (token == ".data")
        {
            inDataSegment = true;
            inTextSegment = false;
            address = 0x10000000; // Start of the data segment at 0x10000000
            continue;
        }

        if (inTextSegment)
        {
            // Handle instructions in the .text section
            string inst, rd, rs1, rs2, label;
            int imm;

            iss >> inst;

            if (opcodeMap.find(inst) != opcodeMap.end())
            {
                if (inst == "beq" || inst == "bne" || inst == "bge" || inst == "blt")
                {
                    // Handle branch instructions
                    iss >> rs1 >> rs2 >> label;

                    // Calculate the branch offset
                    int targetAddress = labelAddresses[label];
                    int offset = (targetAddress - address) / 2; // Offset in terms of instruction units (4 bytes)

                    bitset<32> machineCode(parseSBFormat(inst, rs1, rs2, offset));
                    outFile << "0x" << hex << address << " 0x" << setw(8) << setfill('0')
                            << stoul(machineCode.to_string(), nullptr, 2)
                            << " , " << line << " # " << opcodeMap[inst] << "-" << funct3Map[inst]
                            << "-" << funct7Map[inst] << "-" << registerToBinary(rs1) << "-"
                            << registerToBinary(rs2) << "-" << offset << endl;
                }
                else
                {
                    // Handle other instructions (R, I, S, U, UJ)
                    if (funct7Map.find(inst) != funct7Map.end())
                    {
                        iss >> rd >> rs1 >> rs2;
                        bitset<32> machineCode(parseRFormat(inst, rd, rs1, rs2));
                        outFile << "0x" << hex << address << " 0x" << setw(8) << setfill('0')
                                << stoul(machineCode.to_string(), nullptr, 2)
                                << " , " << line << endl;
                    }
                    else
                    {
                        iss >> rd >> rs1 >> imm;
                        bitset<32> machineCode(parseIFormat(inst, rd, rs1, imm));
                        outFile << "0x" << hex << address << " 0x" << setw(8) << setfill('0')
                                << stoul(machineCode.to_string(), nullptr, 2)
                                << " , " << line << endl;
                    }
                }
                address += 4;
            }
        }
        else if (inDataSegment)
        {
            // Handle data segment
            if (line.find(".byte") != string::npos)
            {
                int value;
                while (iss >> value)
                {
                    outFile << "0x" << hex << address << " 0x" << setw(8) << setfill('0')
                            << value << " , .byte " << value << endl;
                    address += 1; // 1 byte per value
                }
            }
            else if (line.find(".half") != string::npos)
            {
                int value;
                while (iss >> value)
                {
                    outFile << "0x" << hex << address << " 0x" << setw(8) << setfill('0')
                            << value << " , .half " << value << endl;
                    address += 2; // 2 bytes per value
                }
            }
            else if (line.find(".word") != string::npos)
            {
                int value;
                while (iss >> value)
                {
                    outFile << "0x" << hex << address << " 0x" << setw(8) << setfill('0')
                            << value << " , .word " << value << endl;
                    address += 4; // 4 bytes per value
                }
            }
            else if (line.find(".double") != string::npos)
            {
                long long value;
                while (iss >> value)
                {
                    outFile << "0x" << hex << address << " 0x" << setw(16) << setfill('0')
                            << value << " , .double " << value << endl;
                    address += 8; // 8 bytes per value
                }
            }
            else if (line.find(".asciiz") != string::npos)
            {
                string str;
                getline(iss, str);
                for (char c : str)
                {
                    outFile << "0x" << hex << address << " 0x" << setw(8) << setfill('0')
                            << (int)c << " , .asciiz " << c << endl;
                    address += 1; // 1 byte per char
                }
                outFile << "0x" << hex << address << " 0x" << setw(8) << setfill('0')
                        << 0 << " , .asciiz \\0" << endl;
                address += 1; // Null terminator
            }
        }
    }

    inFile.close();
    outFile.close();
}

// Main driver
int main()
{
    // First pass: Gather labels and directives
    firstPass("input.asm");

    // Second pass: Generate machine code and handle labels
    secondPass("input.asm", "output.mc");

    cout << "Assembly translation complete. Check output.mc" << endl;
    return 0;
}
