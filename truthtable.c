#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_VARS 10000
#define MAX_INPUTS 200
#define MAX_GATES 10000
#define MAX_NAME_LEN 17

typedef enum {
    GATE_AND, GATE_OR, GATE_NAND, GATE_NOR, GATE_XOR,
    GATE_NOT, GATE_PASS, GATE_DECODER, GATE_MULTIPLEXER
} GateType;

typedef struct {
    GateType type;
    int size;
    int *params;
} Gate;

typedef struct {
    int input_count;
    int output_count;
    int var_count;
    int inputs[MAX_INPUTS];
    int outputs[MAX_INPUTS];
    Gate gates[MAX_GATES];
    int gate_count;
    char variableNames[MAX_VARS][MAX_NAME_LEN];
    int eval_order[MAX_GATES];
    int eval_order_size;
} Circuit;

static Circuit circuit;
static int values[MAX_VARS];
static int is_input[MAX_VARS] = {0};
static int var_to_gate[MAX_VARS];

static int zeroVar = -1;
static int oneVar = -1;

static int getOrCreateVariable(const char *name) {
    if (strcmp(name, "0") == 0) {
        if (zeroVar == -1) {
            zeroVar = circuit.var_count;
            strcpy(circuit.variableNames[circuit.var_count++], "0");
        }
        return zeroVar;
    }
    if (strcmp(name, "1") == 0) {
        if (oneVar == -1) {
            oneVar = circuit.var_count;
            strcpy(circuit.variableNames[circuit.var_count++], "1");
        }
        return oneVar;
    }
    if (strcmp(name, "_") == 0) {
        int idx = circuit.var_count;
        strcpy(circuit.variableNames[circuit.var_count++], "_discard");
        return idx;
    }
    for (int i = 0; i < circuit.var_count; i++) {
        if (strcmp(circuit.variableNames[i], name) == 0) return i;
    }
    strcpy(circuit.variableNames[circuit.var_count], name);
    return circuit.var_count++;
}

static void parseGate(FILE *file, GateType type) {
    Gate gate;
    gate.type = type;
    gate.size = 0;
    gate.params = NULL;

    if (type == GATE_NOT || type == GATE_PASS) {
        gate.params = malloc(sizeof(int) * 2);
        char in[MAX_NAME_LEN], out[MAX_NAME_LEN];
        fscanf(file, "%16s %16s", in, out);
        gate.params[0] = getOrCreateVariable(in);
        gate.params[1] = getOrCreateVariable(out);
    } else if (type == GATE_AND || type == GATE_OR || type == GATE_XOR ||
               type == GATE_NAND || type == GATE_NOR) {
        gate.params = malloc(sizeof(int) * 3);
        char in1[MAX_NAME_LEN], in2[MAX_NAME_LEN], out[MAX_NAME_LEN];
        fscanf(file, "%16s %16s %16s", in1, in2, out);
        gate.params[0] = getOrCreateVariable(in1);
        gate.params[1] = getOrCreateVariable(in2);
        gate.params[2] = getOrCreateVariable(out);
    } else if (type == GATE_DECODER) {
        int n;
        fscanf(file, "%d", &n);
        gate.size = n;
        int total = n + (1 << n);
        gate.params = malloc(sizeof(int) * total);
        for (int i = 0; i < total; i++) {
            char name[MAX_NAME_LEN];
            fscanf(file, "%16s", name);
            gate.params[i] = getOrCreateVariable(name);
        }
    } else if (type == GATE_MULTIPLEXER) {
        int n;
        fscanf(file, "%d", &n);
        gate.size = n;
        int total = (1 << n) + n + 1;
        gate.params = malloc(sizeof(int) * total);
        for (int i = 0; i < total; i++) {
            char name[MAX_NAME_LEN];
            fscanf(file, "%16s", name);
            gate.params[i] = getOrCreateVariable(name);
        }
    }

    circuit.gates[circuit.gate_count++] = gate;
}

static void buildEvaluationOrder() {
    memset(var_to_gate, -1, sizeof(var_to_gate));

    for (int i = 0; i < circuit.gate_count; i++) {
        Gate *g = &circuit.gates[i];
        int *out = NULL, out_count = 0;
        switch (g->type) {
            case GATE_NOT: case GATE_PASS: out = &g->params[1]; out_count = 1; break;
            case GATE_AND: case GATE_OR: case GATE_XOR: case GATE_NAND: case GATE_NOR:
                out = &g->params[2]; out_count = 1; break;
            case GATE_DECODER:
                out = &g->params[g->size]; out_count = (1 << g->size); break;
            case GATE_MULTIPLEXER:
                out = &g->params[(1 << g->size) + g->size]; out_count = 1; break;
            default: break;
        }
        for (int j = 0; j < out_count; j++) {
            int var = out[j];
            if (strcmp(circuit.variableNames[var], "_discard") == 0) continue;
            var_to_gate[var] = i;
        }
    }

    int **adj = malloc(sizeof(int*) * MAX_GATES);
    for (int i = 0; i < MAX_GATES; i++) adj[i] = calloc(MAX_GATES, sizeof(int));
    int adj_size[MAX_GATES] = {0}, in_deg[MAX_GATES] = {0};

    for (int i = 0; i < circuit.gate_count; i++) {
        Gate *g = &circuit.gates[i];
        int *in = NULL, in_count = 0;
        switch (g->type) {
            case GATE_NOT: case GATE_PASS: in = &g->params[0]; in_count = 1; break;
            case GATE_AND: case GATE_OR: case GATE_XOR: case GATE_NAND: case GATE_NOR:
                in = g->params; in_count = 2; break;
            case GATE_DECODER: in = g->params; in_count = g->size; break;
            case GATE_MULTIPLEXER: in = g->params; in_count = (1 << g->size) + g->size; break;
            default: break;
        }

        for (int j = 0; j < in_count; j++) {
            int var = in[j];
            if (is_input[var] || strcmp(circuit.variableNames[var], "0") == 0 ||
                strcmp(circuit.variableNames[var], "1") == 0 || 
                strcmp(circuit.variableNames[var], "_discard") == 0) continue;

            int prod = var_to_gate[var];
            adj[prod][adj_size[prod]++] = i;
            in_deg[i]++;
        }
    }

    int queue[MAX_GATES], front = 0, rear = 0;
    circuit.eval_order_size = 0;
    for (int i = 0; i < circuit.gate_count; i++) if (!in_deg[i]) queue[rear++] = i;

    while (front < rear) {
        int u = queue[front++];
        circuit.eval_order[circuit.eval_order_size++] = u;
        for (int i = 0; i < adj_size[u]; i++) {
            int v = adj[u][i];
            if (--in_deg[v] == 0) queue[rear++] = v;
        }
    }

    if (circuit.eval_order_size != circuit.gate_count) {
        fprintf(stderr, "Cycle detected\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < MAX_GATES; i++) free(adj[i]);
    free(adj);
}

static void evaluateGate(const Gate *g) {
    switch (g->type) {
        case GATE_NOT: values[g->params[1]] = !values[g->params[0]]; break;
        case GATE_PASS: values[g->params[1]] = values[g->params[0]]; break;
        case GATE_AND: values[g->params[2]] = values[g->params[0]] & values[g->params[1]]; break;
        case GATE_OR: values[g->params[2]] = values[g->params[0]] | values[g->params[1]]; break;
        case GATE_XOR: values[g->params[2]] = values[g->params[0]] ^ values[g->params[1]]; break;
        case GATE_NAND: values[g->params[2]] = !(values[g->params[0]] & values[g->params[1]]); break;
        case GATE_NOR: values[g->params[2]] = !(values[g->params[0]] | values[g->params[1]]); break;
        case GATE_DECODER: {
            int in = 0;
            for (int i = 0; i < g->size; i++) in = (in << 1) | values[g->params[i]];
            for (int i = 0; i < (1 << g->size); i++) values[g->params[g->size + i]] = (i == in);
            break;
        }
        case GATE_MULTIPLEXER: {
            int sel = 0;
            for (int i = 0; i < g->size; i++) sel = (sel << 1) | values[g->params[(1 << g->size) + i]];
            values[g->params[(1 << g->size) + g->size]] = values[g->params[sel]];
            break;
        }
    }
}

static void printTruthRow() {
    for (int i = 0; i < circuit.input_count; i++) printf("%d ", values[circuit.inputs[i]]);
    printf("| ");
    for (int i = 0; i < circuit.output_count; i++) {
        printf("%d", values[circuit.outputs[i]]);
        if (i < circuit.output_count - 1) printf(" ");
    }
    putchar('\n');
}

static void generateTruthTable() {
    memset(values, 0, sizeof(values));
    if (zeroVar != -1) values[zeroVar] = 0;
    if (oneVar != -1) values[oneVar] = 1;

    for (int row = 0; row < (1 << circuit.input_count); row++) {
        for (int i = 0; i < circuit.input_count; i++)
            values[circuit.inputs[i]] = (row >> (circuit.input_count - 1 - i)) & 1;
        for (int i = 0; i < circuit.eval_order_size; i++)
            evaluateGate(&circuit.gates[circuit.eval_order[i]]);
        printTruthRow();
    }
}

static void parseCircuitFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) { perror("Error opening file"); exit(EXIT_FAILURE); }

    circuit = (Circuit){0}; zeroVar = oneVar = -1;
    char directive[32];

    fscanf(file, "%16s %d", directive, &circuit.input_count);
    for (int i = 0; i < circuit.input_count; i++) {
        char name[MAX_NAME_LEN]; fscanf(file, "%16s", name);
        int var = getOrCreateVariable(name);
        circuit.inputs[i] = var; is_input[var] = 1;
    }

    fscanf(file, "%16s %d", directive, &circuit.output_count);
    for (int i = 0; i < circuit.output_count; i++) {
        char name[MAX_NAME_LEN]; fscanf(file, "%16s", name);
        circuit.outputs[i] = getOrCreateVariable(name);
    }

    while (fscanf(file, "%16s", directive) == 1) {
        if (strcmp(directive, "NOT") == 0) parseGate(file, GATE_NOT);
        else if (strcmp(directive, "AND") == 0) parseGate(file, GATE_AND);
        else if (strcmp(directive, "OR") == 0) parseGate(file, GATE_OR);
        else if (strcmp(directive, "XOR") == 0) parseGate(file, GATE_XOR);
        else if (strcmp(directive, "NAND") == 0) parseGate(file, GATE_NAND);
        else if (strcmp(directive, "NOR") == 0) parseGate(file, GATE_NOR);
        else if (strcmp(directive, "PASS") == 0) parseGate(file, GATE_PASS);
        else if (strcmp(directive, "DECODER") == 0) parseGate(file, GATE_DECODER);
        else if (strcmp(directive, "MULTIPLEXER") == 0) parseGate(file, GATE_MULTIPLEXER);
        else { fprintf(stderr, "Invalid directive: %s\n", directive); exit(EXIT_FAILURE); }
    }

    fclose(file);
    buildEvaluationOrder();
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <circuit_file>\n", argv[0]);
        return 1;
    }
    parseCircuitFile(argv[1]);
    generateTruthTable();
    return 0;
}