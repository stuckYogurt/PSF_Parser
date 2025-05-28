#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define uns unsigned
#define MAX_INPUT_LENGTH 4096

// using unbalanced binary search tree to store PSF summatives' members
struct treeNode {
    char varName;
    int freq; // signed because signals non-int division

    struct treeNode* leftChild;
    struct treeNode* rightChild;
};

// the beginning of every Summative
struct PSF_Summative {
    int coeff;
    struct treeNode* map_of_vars;
};

struct PSF_Summative init_Summative(int coeff) {
    struct PSF_Summative root;
    root.coeff = coeff;
    root.map_of_vars = 0;
    return root;
}

struct treeNode* init_treeNode(const char varName) {
    struct treeNode* ret = malloc(sizeof(struct treeNode));

    ret->varName = varName;
    ret->freq = 1;

    ret->leftChild = 0;
    ret->rightChild = 0;

    return ret;
}

struct treeNode* getNodeParent(struct treeNode* root, const struct treeNode* target) {
    if (root == 0 || root == target) return 0;

    if (root->leftChild == target || root->rightChild == target) {
        return root;
    }

    struct treeNode* resp1 = getNodeParent(root->leftChild, target);
    if (resp1 != NULL) return resp1;

    struct treeNode* resp2 = getNodeParent(root->rightChild, target);
    if (resp2 != NULL) return resp2;

    return NULL;
}

// deleting element but leaving children in place
void erase_treeNode(struct PSF_Summative* summative, struct treeNode* target) {
    struct treeNode* root = summative->map_of_vars;
    // simplest cases: one child absent
    if (!target->rightChild && target->leftChild) {
        struct treeNode* temp = target->leftChild;
        memcpy(target,target->leftChild, sizeof(struct treeNode));

        free(temp);
        return;
    }

    if (!target->leftChild && target->rightChild) {
        struct treeNode* temp = target->rightChild;
        memcpy(target,target->rightChild, sizeof(struct treeNode));

        free(temp);
        return;
    }


    struct treeNode* parent = getNodeParent(root, target);

    if (!parent) {
        if (!target->leftChild && !target->rightChild) {
            free(summative->map_of_vars);
            summative->map_of_vars = 0;
            return;
        }

        if (!target->rightChild) {
            struct treeNode* tempLeft = target->leftChild;
            struct treeNode* tempRight = target->rightChild;

            memcpy(target, target->leftChild, sizeof(struct treeNode));
            free(tempLeft);

            struct treeNode* maxChild = target;
            while (maxChild->rightChild)
                maxChild = maxChild->rightChild;

            maxChild->rightChild = tempRight;
            return;
        }

        struct treeNode* tempLeft = target->leftChild;
        struct treeNode* tempRight = target->rightChild;

        memcpy(target, target->rightChild, sizeof(struct treeNode));
        free(tempRight);

        struct treeNode* minChild = target;
        while (minChild->leftChild)
            minChild = minChild->leftChild;

        minChild->leftChild = tempLeft;
        return;
    }

    // for better debug
    if (parent->rightChild != target && parent->leftChild != target)
        fprintf(stderr, "Node parent found incorrectly\n");


    if (!target->leftChild && !target->rightChild) {
        free(target);
        if (parent->leftChild == target)
            parent->leftChild = 0;
        else
            parent->rightChild = 0;

        return;
    }

    if (parent->rightChild == target) {
        parent->rightChild = target->rightChild;

        struct treeNode* minChild = target->rightChild;
        while (minChild->leftChild)
            minChild = minChild->leftChild;

        minChild->leftChild = target->leftChild;
        free(target);
        return;
    }

    parent->leftChild = target->rightChild;

    struct treeNode* minChild = target->rightChild;
    while (minChild->leftChild)
        minChild = minChild->leftChild;

    minChild->leftChild = target->leftChild;
    free(target);

}

// append a var into Summative with positive (multiply) or negative (divide) degree (frequency)
void emplace_treeNode(struct PSF_Summative* summative, const char varName, const int freq) {
    struct treeNode* tree_iter = summative->map_of_vars;

    while (1) {
        if (tree_iter->varName == varName) {
            tree_iter->freq += freq;

            if (tree_iter->freq == 0)
                erase_treeNode(summative, tree_iter);

            return;
        }

        if (tree_iter->varName > varName) {
            if (tree_iter->leftChild == 0) {
                tree_iter->leftChild = init_treeNode(varName);
                tree_iter->leftChild->freq = freq;
                return;
            }

            tree_iter = tree_iter->leftChild;
            continue;
        }

        // if (tree_iter->varName < varName)
        if (tree_iter->rightChild == 0) {
            tree_iter->rightChild = init_treeNode(varName);
            tree_iter->rightChild->freq = freq;
            return;
        }

        tree_iter = tree_iter->rightChild;

    }
}

void emplace_treeNode_inSummative(struct PSF_Summative* root, const char varName, const int freq) {
    if (freq == 0) return;

    if (root->map_of_vars == 0) {
        root->map_of_vars = init_treeNode(varName);
        root->map_of_vars->freq = freq;
        return;
    }

    emplace_treeNode(root, varName, freq);

}

// frequency of var in summative: frequency of x in x*x*y*x is 3
uns get_freqOfVar(struct treeNode* map_root, const char varName) {
    struct treeNode* map_iter = map_root;
    while (map_iter != 0) {
        if (map_iter->varName == varName)
            return map_iter->freq;

        if (map_iter->varName > varName) {
            map_iter = map_iter->leftChild;
            continue;
        }

        // if (map_iter->varName < varName)
        map_iter = map_iter->rightChild;
    }

    return 0;
}


// if summatives could make a sum, e.g. x*y + 2*y*x = 3*x*y (true)
bool are_summableSummatives_node(struct treeNode* memberSummative, struct PSF_Summative summative2) {
    if (!memberSummative)
        return true;

    if (memberSummative->freq != get_freqOfVar(summative2.map_of_vars, memberSummative->varName))
        return false;

    return are_summableSummatives_node(memberSummative->leftChild, summative2) && are_summableSummatives_node(memberSummative->rightChild, summative2);
}

bool are_summableSummatives(struct PSF_Summative summative1, struct PSF_Summative summative2) {
    return are_summableSummatives_node(summative1.map_of_vars, summative2) && are_summableSummatives_node(summative2.map_of_vars, summative1);
}


uns charFrequencyInString(const char* str, const char ch) {
    uns freq = 0;
    for (uns i = 0; str[i] != '\0'; i++)
        if (str[i] == ch)
            freq++;

    return freq;
}



bool isDigit(const char c) {
    if ((int)c >= 48 && (int)c <= 57)
        return true;

    return false;
}

// coeffiecient's sign is defined in string parser
uns getUnsFromString(const char* str) {
    uns res = 0;

    // ensure string and digit sequence don't end
    for (uns i = 0; str[i] != '\0' && isDigit(str[i]); i++)
        res = res*10 + ((int)(str[i]) - 48);

    return res;
}


int StartsWithMinus(const char* inputStr) {
    for (uns i = 0; inputStr[i] != '\0' && (inputStr[i] == ' ' || inputStr[i] == '-'); i++)
        if (inputStr[i] == '-')
            return 0;

    return 1;
}

// Product-Sum Form
struct PSF {
    struct PSF_Summative* summative; // to first summative
    uns len;
};

void print_PSFSummativeMember(const struct treeNode* s, bool startWithMultiplication) {
    if (!s) return;

    if (startWithMultiplication)
        printf("*");
    for (uns i = 0; i < s->freq; i++) {
        if (i == s->freq - 1) {
            printf("%c", s->varName);
            break;
        }
        printf("%c*", s->varName);
    }

    print_PSFSummativeMember(s->leftChild, true);
    print_PSFSummativeMember(s->rightChild, true);
}

void print_PSFSummative(const struct PSF_Summative* s) {
    if (abs(s->coeff) != 1) {
        printf("%i", abs(s->coeff));
        print_PSFSummativeMember(s->map_of_vars, true);
    } else
        print_PSFSummativeMember(s->map_of_vars, false);
}

void print_PSF(const struct PSF form) {
    if (form.len == 0) {
        printf("0");
        return;
    }

    for (uns i = 0; i < form.len; i++) {
        if (i == 0 && form.summative[i].coeff < 0)
            printf("-");

        if (form.summative[i].map_of_vars)
            print_PSFSummative(form.summative + i);
        else
            printf("%i", form.summative[i].coeff);

        if (i < form.len - 1) {
            if (form.summative[i + 1].coeff >= 0)
                printf(" + ");
            else
                printf(" - ");
        }
    }
}


void free_PSFSummativeMember(struct treeNode* target) {
    if (!target) return;

    free_PSFSummativeMember(target->leftChild);
    free_PSFSummativeMember(target->rightChild);

    free(target);
}

void free_PSFSummative(struct PSF_Summative* target) {
    if (!target) return;

    free_PSFSummativeMember(target->map_of_vars);
    target->map_of_vars = 0;
    // free(target);
}

void free_PSF(struct PSF* target) {
    if (!target) return;

    for (uns i = 0; i < target->len; i++) {
        free_PSFSummative(target->summative + i);
    }

    free(target->summative);
}

// empty string consist only of spaces, zeros or has no elements
bool isEmptyString(const char* inputStr) {
    bool isEmpty = true;
    for (uns sliderInd = 0; inputStr[sliderInd] != '\0' && isEmpty; sliderInd++)
        isEmpty = inputStr[sliderInd] == ' ' || inputStr[sliderInd] == '0';

    return isEmpty;
}

// parsing a string to get PSF
struct PSF getPSF(const char* inputStr) {
    if (isEmptyString(inputStr)) {
        struct PSF ret;
        ret.summative = 0;
        ret.len = 0;
        return ret;
    }

    // if starts with minus, it's prefix, not infix
    uns summativeCount =
            charFrequencyInString(inputStr, '+') +
            charFrequencyInString(inputStr, '-') +
            StartsWithMinus(inputStr);

    struct PSF_Summative* summatives = malloc(sizeof(struct PSF_Summative) * summativeCount);

    for (uns i = 0; i < summativeCount; i++)
        summatives[i] = init_Summative(1);

    uns summativeCurrInd = 0;
    bool isFirstSymbol = true; // if '-' encountered is the first non-space char
    for (uns sliderInd = 0; inputStr[sliderInd] != '\0'; sliderInd++) {
        if (inputStr[sliderInd] == ' ' || inputStr[sliderInd] == '*')
            continue;

        // making new summative if '+' or '-' encountered
        if (inputStr[sliderInd] == '+') {
            isFirstSymbol = false;
            summativeCurrInd++;
            continue;
        }

        if (inputStr[sliderInd] == '-') {
            // prefix or infix
            if (isFirstSymbol) {
                isFirstSymbol = false;
            } else
                summativeCurrInd++;

            summatives[summativeCurrInd].coeff = -1;
            continue;
        }

        if (isDigit(inputStr[sliderInd])) {
            isFirstSymbol = false;
            summatives[summativeCurrInd] = init_Summative(
                    (int)getUnsFromString(inputStr + sliderInd) * summatives[summativeCurrInd].coeff         // since coeff -1 or 1
            );

            // send to the end of number
            uns endOfNum = sliderInd + 1;
            while (isDigit(inputStr[endOfNum]))
                endOfNum++;

            sliderInd = endOfNum - 1;
            continue;
        }

        // if not +, *, space, then we have a variable
        emplace_treeNode_inSummative(summatives + summativeCurrInd, inputStr[sliderInd], 1);
        isFirstSymbol = false;
    }

    struct PSF ret;
    ret.len = summativeCount;
    ret.summative = summatives;

    return ret;
}


void deepcpy_PSFSummativeMembers(struct treeNode* destination, struct treeNode* target) {
    destination->varName = target->varName;
    destination->freq = target->freq;

    if (target->leftChild) {
        destination->leftChild = malloc(sizeof(struct treeNode));
        deepcpy_PSFSummativeMembers(destination->leftChild, target->leftChild);
    } else
        destination->leftChild = 0;

    if (target->rightChild) {
        destination->rightChild = malloc(sizeof(struct treeNode));
        deepcpy_PSFSummativeMembers(destination->rightChild, target->rightChild);
    } else
        destination->rightChild = 0;

}

struct PSF_Summative deepcpy_PSFSummative(struct PSF_Summative* target) {
    struct PSF_Summative destination;

    destination = init_Summative(target->coeff);

    if (target->map_of_vars) {
        destination.map_of_vars = malloc(sizeof(struct treeNode));
        deepcpy_PSFSummativeMembers(destination.map_of_vars, target->map_of_vars);
    } else
        destination.map_of_vars = 0;

    return destination;
}

// desLen is used if more memory needed to avoid using realloc()
struct PSF deepcpy_PSF(const struct PSF target, uns desLen) {
    struct PSF destination = target;
    destination.summative = malloc(sizeof(struct PSF_Summative) * desLen);

    for (uns i = 0; i < target.len; i++)
        destination.summative[i] = deepcpy_PSFSummative(target.summative + i);

    return destination;
}


// summant1 + summant2
struct PSF sumPSF(const struct PSF summant1, const struct PSF summant2) {
    struct PSF_Summative* deviantSummants[summant2.len]; // summants that are abscent in summant1
    uns currInd_deviantSummants = 0;

    struct PSF retSum = deepcpy_PSF(summant1, summant1.len + summant2.len);
    for (uns indOfSummants2 = 0; indOfSummants2 < summant2.len; indOfSummants2++) {
        deviantSummants[currInd_deviantSummants] = summant2.summative + indOfSummants2;
        currInd_deviantSummants++;

        for (uns indOfSummants1 = 0; indOfSummants1 < retSum.len; indOfSummants1++)
            // if it's common summant
            if (are_summableSummatives(retSum.summative[indOfSummants1], summant2.summative[indOfSummants2])) {
                currInd_deviantSummants--; // matching summative must be on top of deviantSummants

                if (retSum.summative[indOfSummants1].coeff + summant2.summative[indOfSummants2].coeff == 0) {
                    // if 0, erasing summative
                    free_PSFSummative(retSum.summative +indOfSummants1);

                    memmove(retSum.summative + indOfSummants1 ,
                            retSum.summative + indOfSummants1 + 1,
                            sizeof(struct PSF_Summative) * (retSum.len - indOfSummants1 - 1));

                    retSum.len--;

                } else
                    retSum.summative[indOfSummants1].coeff += summant2.summative[indOfSummants2].coeff;

                break;
            }

    }

    // deviants insertion
    for (uns i = 0; i < currInd_deviantSummants; i++) {
        if (deviantSummants[i]->coeff != 0) {
            retSum.summative[retSum.len] = deepcpy_PSFSummative(deviantSummants[i]);
            retSum.len ++;
        }
    }

    // realloc is undefined if resite to 0
    if (retSum.len == 0) {
        free_PSF(&retSum);
        return getPSF("");
    }

    void* tmp = realloc(retSum.summative, sizeof(struct PSF_Summative) * retSum.len);
    if (NULL == tmp)
        fprintf(stderr, "\nReallocation failed (new size: %i, old size: %i), proceeding w/o...\n", retSum.len, summant1.len + summant2.len);
    else
        retSum.summative = tmp;

    return retSum;
}

void multiplyByMembers(struct PSF_Summative* target, const struct treeNode* multiplier) {
    if (!target || !multiplier) return;

    emplace_treeNode_inSummative(target, multiplier->varName, multiplier->freq);

    multiplyByMembers(target, multiplier->leftChild);
    multiplyByMembers(target, multiplier->rightChild);

}

void multiplyBySummative(struct PSF* target, const struct PSF_Summative* multiplier) {
    for (uns i = 0; i < target->len; i++) {
        target->summative[i].coeff *= multiplier->coeff;

        if (!multiplier->map_of_vars) continue;
        multiplyByMembers(target->summative + i, multiplier->map_of_vars);
    }
}

struct PSF multiplyPSFs(struct PSF target1, const struct PSF target2) {
    struct PSF result;
    result.summative = 0;
    result.len = 0;

    struct PSF temp;
    struct PSF temp1;

    for (uns multiplierElemInd = 0; multiplierElemInd < target2.len; multiplierElemInd++) {
        temp = deepcpy_PSF(target1, target1.len);
        multiplyBySummative(&temp, target2.summative + multiplierElemInd);

        temp1 = sumPSF(result, temp);
        free_PSF(&result);
        result = temp1;

        free_PSF(&temp);
    }

    return result;
}

// target1 + (-1) * target2
struct PSF subtractPSF(struct PSF target1, struct PSF target2) {
    struct PSF minusOne = getPSF("-1");
    struct PSF subtrahend = multiplyPSFs(target2, minusOne);
    free_PSF(&minusOne);

    struct PSF ret = sumPSF(target1, subtrahend);
    free_PSF(&subtrahend);

    return ret;
}

// target1 == target2 <=> target1 - target2 == 0
bool isEqualPSF(struct PSF target1, struct PSF target2) {
    struct PSF subtruction = subtractPSF(target1, target2);

    bool result = subtruction.len == 0;
    free_PSF(&subtruction);
    return (result);
}

// e.g. no x^-1 present
bool isIntVars(struct treeNode* target) {
    if (!target) return true;

    return target->freq > 0 && isIntVars(target->leftChild) && isIntVars(target->rightChild);
}

bool isIntVars_PSF(struct PSF target) {
    bool result = true;
    for (uns i = 0; i < target.len; i++)
        result &= isIntVars(target.summative[i].map_of_vars);

    return result;
}

// 1 / target
void makeDenominator(struct treeNode* target) {
    if (!target) return;

    target->freq *= -1;

    makeDenominator(target->leftChild);
    makeDenominator(target->rightChild);
}

struct DivRet {
    bool isCorrect; // if no errors encountered
    struct PSF form;
};

// returns PSF{0, 0} if undefined division
struct DivRet dividePSF(struct PSF target1, struct PSF target2) {
    if (target1.len == 0)
        return (struct DivRet){true, getPSF("")};

    if (target2.len != 1) {
//        fprintf(stderr, "Undefined behaviour: Incorrect denominator detected\n");
        return (struct DivRet){false, getPSF("")};
    }

    struct PSF denominator = deepcpy_PSF(target2, target2.len);
    for (uns i = 0; i < denominator.len; i++) {
        makeDenominator(denominator.summative[i].map_of_vars);
    }


    struct PSF result = multiplyPSFs(target1, denominator);
    free_PSF(&denominator);

    if (!isIntVars_PSF(result)) {
//        fprintf(stderr, "Undefined behaviour: Incorrect denominator detected\n");
        free_PSF(&result);
        return (struct DivRet){false, getPSF("")};
    }

    // dividing coefficients twice (extra was performed in multiplyPSFs)
    for (uns i = 0; i < result.len; i++)
        result.summative[i].coeff = result.summative[i].coeff / target2.summative->coeff / target2.summative->coeff;


    return (struct DivRet){true, result};
}

// choose what to do with forms
void chooseOperation(const char oper, struct PSF* f1, struct PSF* f2) {
    if (oper == '-') {
        struct PSF res = subtractPSF(*f1,*f2);
        print_PSF(res);
        printf("\n");

        free_PSF(&res);
        return;
    }
    if (oper == '+') {
        struct PSF res = sumPSF(*f1,*f2);
        print_PSF(res);
        printf("\n");

        free_PSF(&res);
        return;
    }
    if (oper == '*') {
        struct PSF res = multiplyPSFs(*f1,*f2);
        print_PSF(res);
        printf("\n");

        free_PSF(&res);
        return;
    }
    if (oper == '=') {
        bool res = isEqualPSF(*f1,*f2);
        printf(res ? "equal" : "not equal");
        printf("\n\n");

        return;
    }
    if (oper == '/') {
        struct DivRet res = dividePSF(*f1,*f2);
        if (!res.isCorrect)
            printf("error");
        else
            print_PSF(res.form);

        printf("\n");

        free_PSF(&res.form);
        return;
    }
}

// PSF created and freed here
void liveOperations(const char oper, char* _f1, char* _f2) {
    struct PSF f1 = getPSF(_f1);
    struct PSF f2 = getPSF(_f2);

    chooseOperation(oper, &f1, &f2);

    free_PSF(&f1);
    free_PSF(&f2);
}


int main() {
    char operatorStr[MAX_INPUT_LENGTH];
    char form1[MAX_INPUT_LENGTH];
    char form2[MAX_INPUT_LENGTH];

    printf("To exit, type \"exit\"\n");
    while (1) {
        if (fgets(operatorStr, sizeof(char) * MAX_INPUT_LENGTH, stdin) == NULL) {
            return 0;
        };
        operatorStr[strcspn(operatorStr, "\r\n")] = 0;

        if (strcmp(operatorStr, "exit") == 0)
            break;

        // check if input has defined operator
        if (strlen(operatorStr) != 1 || strlen(operatorStr) == 1 && strcspn(operatorStr, "+-/*=") == 1) {
            printf("Issue reading operation sign, please try again\n");
            continue;
        }

        if (fgets(form1, sizeof(char) * MAX_INPUT_LENGTH, stdin) == NULL) {
            return 0;
        }
        form1[strcspn(form1, "\r\n")] = 0;

        if (fgets(form2, sizeof(char) * MAX_INPUT_LENGTH, stdin) == NULL) {
            return 0;
        }
        form2[strcspn(form2, "\r\n")] = 0;

        liveOperations(operatorStr[0], form1, form2);
    }


    return 0;
}