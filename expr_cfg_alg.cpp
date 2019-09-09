#include <sstream>
#include <string>
#include "expr_cfg.h"

extern CFGNodePool cfg_pool;
int label_count = 0;

std::string gv_escape(const std::string& s) {
    std::ostringstream ss;

    for (auto ch : s) {
        if (ch == '\"') {
            ss << "\\\"";
        } else if (ch == '\n') {
            ss << "\\l";
        } else {
            ss << ch;
        }
    }

    return ss.str();
}

static inline void nodeToString(const char *tabs, CFGNode *n) {
    if (n->isA(CFGStmt_kind)) {
        CFGStmt *s = static_cast<CFGStmt *>(n);

        if (s->label.empty()) {
            std::cout << tabs << s->toString() << '\n';
        } else {
            std::cout << s->label << ":" << tabs << s->toString() << '\n';
        }   
    } else {
        std::cout << n->toString();
    }
}

static inline void printStmtInfo(const char *tabs, CFGStmt *s) {
    if (s->label.empty()) {
        std::cout << tabs << s->toString() << '\n';
    } else {
        std::cout << s->label << ":" << tabs << s->toString() << '\n';
    }   
    
    if (s->isA(CFGCBRStmt_kind)) {
        CFGCBRStmt *cbrs = static_cast<CFGCBRStmt *>(s);
        std::cout  << " " << s->pred_count
                   << " " << std::hex << cbrs->target_true->label << std::dec
                   << " " << std::hex << cbrs->target_false->label << std::dec
                   << '\n';
    } else if (s->isA(CFGSimpleStmt_kind)) {
        CFGSimpleStmt *ss = static_cast<CFGSimpleStmt *>(s);

        std::cout << ", Pred: " << ss->pred_count;
        if (ss->next != nullptr && !ss->next->label.empty()) {
            std::cout << ", " << ss->next->label;
        }
        std::cout << '\n';
    }
}

std::string getAsm(CFGStmt* root, int varsize, int tempsize){
    std::queue<CFGStmt *> q;
    std::unordered_set<CFGStmt *> visited;

    std::string haudi;
    
    q.push(root);
    while (!q.empty()) {
        CFGStmt *s = q.front();
        q.pop();

        if (visited.find(s) != visited.end()) {
            continue;
        }
        if(s->label.length() > 0){
            haudi+= s->label + ":\n";
        }

        haudi+= s->vizto();


        visited.insert(s);
        CFGStmtVector edges = s->getEdges();

        if(edges.size() == 1 && edges[0]->label.length() > 0 ){
            haudi+= "jmp "+edges[0]->label+"\n";
        }

        for (int i = 0; i < edges.size(); i++) {
            q.push(edges[i]);
        }
    }

    std::string retoro = "global main\nextern printf\nsection .text\n\n";
    retoro+="main:\n";
    retoro+= "push ebp\n";
    retoro+= "mov ebp, esp\n";
    retoro+= "sub ebp, "+std::to_string(varsize+tempsize)+"\n";
    retoro+= haudi+"\n";
    retoro+= "main_end:\n";
    retoro+= "leave\n";
    retoro+= "ret\n\n";


    return retoro;
}

void CFGSetLabels(CFGStmt *root) {
    std::queue<CFGStmt *> q;
    std::unordered_set<CFGStmt *> visited;
    
    q.push(root);
    unsigned lbl_count = 0;
    while (!q.empty()) {
        CFGStmt *s = q.front();
        q.pop();

        if (visited.find(s) != visited.end()) {
            continue;
        }
        s->label = "LC" + std::to_string(lbl_count++);
        visited.insert(s);
        CFGStmtVector edges = s->getEdges();

        for (int i = 0; i < edges.size(); i++) {
            q.push(edges[i]);
        }
    }
}


void CFGHVisit(CFGStmt *root) {
    std::queue<CFGStmt *> q;
    std::unordered_set<CFGStmt *> visited;
    
    q.push(root);
    while (!q.empty()) {
        CFGStmt *s = q.front();
        q.pop();

        if (visited.find(s) != visited.end()) {
            continue;
        }
        visited.insert(s);
        CFGStmtVector edges = s->getEdges();

        for (int i = 0; i < edges.size(); i++) {
            std::cout << s->label << " -> " << edges[i]->label << "\n";
            q.push(edges[i]);
        }
    }

    for (auto& stmt : visited) {
        std::cout << stmt->label << " [shape=box,label=\"" 
                  << gv_escape(stmt->toString()) << "\"]\n";
    } 
}

static CFGStmt *CFGMakeBasicBlocksHelper(CFGStmt *root, std::unordered_map<CFGStmt *, CFGStmt *>& visited) {
    if (root == nullptr) {
        return nullptr;
    }
    if (visited.find(root) != visited.end()) {
        return visited[root];
    }
    CFGStmt *newStmt = root;
    CFGStmt *firstStmt = root;
    CFGStmt *lastStmt = root;

    //Try to merge statements
    CFGStmt *next = root->getNext();
    while (next != nullptr && next->pred_count <= 1) {
        lastStmt = next;
        next = next->getNext();
    }

    if (firstStmt != lastStmt) {
        newStmt = cfg_pool.CFGBlockStmtCreate(firstStmt, lastStmt);
        newStmt->pred_count = firstStmt->pred_count;
    }
    visited[root] = newStmt;

    if (lastStmt->isA(CFGSimpleStmt_kind)) {
        CFGSimpleStmt *ss = static_cast<CFGSimpleStmt *>(lastStmt);
        ss->next = CFGMakeBasicBlocksHelper(ss->next, visited);
    } else if (lastStmt->isA(CFGCBRStmt_kind)) {
        CFGCBRStmt *cbrs = static_cast<CFGCBRStmt *>(lastStmt);
        cbrs->target_true = CFGMakeBasicBlocksHelper(cbrs->target_true, visited);
        cbrs->target_false = CFGMakeBasicBlocksHelper(cbrs->target_false, visited);
    }
    return newStmt;
}

CFGStmt *CFGMakeBasicBlocks(CFGStmt *root) {
    std::unordered_map<CFGStmt *, CFGStmt *> visited;

    return CFGMakeBasicBlocksHelper(root, visited);
}

std::string CFGToString(CFGStmt *root) 
{
    std::queue<CFGStmt *> q;
    std::unordered_set<CFGStmt *> visited;
    
    q.push(root);

    std::ostringstream out;
    while (!q.empty()) {
        CFGStmt *s = q.front();
        q.pop();

        if (visited.find(s) != visited.end()) {
            continue;
        }
        visited.insert(s);
        
        if (!s->label.empty()) {
            out << s->label << ": ";
        }
        out << s->toString();
        
        CFGStmtVector edges = s->getEdges();
        if (edges.size() == 1) {
            out << " : goto " << edges[0]->label;
        } else if (edges.size() == 0) {
            out << " : goto end"; 
        }
        out << '\n';

        for (int i = 0; i < edges.size(); i++) {
            q.push(edges[i]);
        }
    }
    out << "end:";

    return out.str();
}

static CFGStmt *CFGRemoveNopsHelper(CFGStmt *root, std::unordered_map<CFGStmt *, CFGStmt *>& visited) {
    if (root == nullptr) {
        return nullptr;
    }
    if (visited.find(root) != visited.end()) {
        return visited[root];
    }

    CFGStmt *stmt = root;
    if (root->isA(CFGNopStmt_kind)) {
        if (root->getNext() == nullptr) {
            visited[root] = root;
            return root;
        }
        do {
            stmt = stmt->getNext();
        } while (stmt->isA(CFGNopStmt_kind) && stmt->getNext() != nullptr);
        
        CFGStmt *stmt2 = root;
        do {
            visited[stmt2] = stmt;
            stmt2 = stmt2->getNext();
        } while (stmt2->isA(CFGNopStmt_kind) && stmt2->getNext() != nullptr);
    }
    visited[root] = stmt;
    if (stmt->isA(CFGSimpleStmt_kind)) {
        CFGSimpleStmt *ss = static_cast<CFGSimpleStmt *>(stmt);
        ss->next = CFGRemoveNopsHelper(ss->next, visited);
    } else if (stmt->isA(CFGCBRStmt_kind)) {
        CFGCBRStmt *cbrs = static_cast<CFGCBRStmt *>(stmt);
        cbrs->target_true = CFGRemoveNopsHelper(cbrs->target_true, visited);
        cbrs->target_false = CFGRemoveNopsHelper(cbrs->target_false, visited);
    }
    return stmt;
}

CFGStmt *CFGRemoveNops(CFGStmt *root) 
{
    std::unordered_map<CFGStmt *, CFGStmt *> visited;

    return CFGRemoveNopsHelper(root, visited);
}