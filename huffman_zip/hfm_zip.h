/*************************************************************************
	> File Name: hfm_zip.h
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年09月22日 星期日 13时27分11秒
 ************************************************************************/

#include "util.h"

int prepare(char *filename);
HuffmanTree *build(int n);
Node *getNewNode();
int compress(HuffmanTree *tree, char *filename, char *destname);
int uncompress(char *filename, char *destname);

void output(Node *root) {
    if (root == NULL) return;
    output(root->lchild);
    printf("%d ", root->p);
    output(root->rchild);
}

int uncompress(char *filename, char *destname) {
    FILE *fw = fopen(destname, "w+");
    FILE *fr = fopen(filename, "r");
    if (fw == NULL || fr == NULL) {
        fclose((fw == NULL ? fw : fr));
        return 0;
    }
    Data data;
    int data_size = sizeof(data);
    int data_cnt = 0;
    arr = (Node *) malloc (sizeof(Node) * 256);
    fread(&data, data_size, 1, fr);
    while (data.num != -1) {
        arr[cnt].ch = (int) data.ch;
        arr[cnt].p = data.num;
        //printf("%d--%d\n", arr[cnt].ch, arr[cnt].p);
        data_cnt += data.num;
        fread(&data, data_size, 1, fr);
        cnt++;
    }
    HuffmanTree *tree = build(cnt);
    Node *tmp = tree->root;
    int ch = fgetc(fr);
    while (ch != EOF) {
        for (int i = 0; i < 8; i++) {
            if ((ch & (1 << i)) == 0) {
                tmp =  tmp->lchild;
            } else {
                tmp = tmp->rchild;
            }
            if (tmp->lchild == NULL && tmp->rchild == NULL) {
                if (data_cnt <= 0) break;
                fputc(tmp->ch, fw);
                data_cnt--;
                tmp = tree->root;
            }
        }
        ch = fgetc(fr);
    }
    fclose(fr);
    fclose(fw);
    return 0;
}

void add_info(char *destname) {
    FILE *fw = fopen(destname, "wb");
    Data data;
    int data_size = sizeof(data);
    for (int i = 0; i < cnt; i++) {
        data.ch = (char) arr[i].ch;
        data.num = arr[i].p;
        fwrite(&data, data_size, 1, fw);
    }
    data.num = -1;
    fwrite(&data, data_size, 1, fw);
    fclose(fw);
}

int compress(HuffmanTree *tree, char *filename, char *destname) {
    add_info(destname);
    FILE *fout = fopen(destname, "ab+");
    FILE *fread = fopen(filename, "r");
    int ch = fgetc(fread);
    unsigned char value = 0;
    int pos = 0;
    char str[100] = {0};
    while (ch != EOF) {
        for (int i = 0; i < tree->n; i++) {
            if (tree->codes[i].ch != ch) continue;
            strcpy(str, tree->codes[i].str);
            break;
        }
        int len = strlen(str);
        for (int i = 0; i < len; i++) {
            if (str[i] == '1') {
                value |= (1 << pos);
            } else if (str[i] == '0') {
                value &= ~(1 << pos);
            } else {
                perror("Error code");
                fclose(fread);
                fclose(fout);
                remove(destname);
                return 1;
            }
            ++pos;
            if (pos == 8) {
                fputc(value, fout);
                value = 0;
                pos = 0;
            }
        }
        ch = fgetc(fread);
    }
    if (pos > 0) {
        //printf("deal pos = %d\n", pos);
        fputc(value, fout);
    }
    fclose(fread);
    fclose(fout);
    return 0;
}

int prepare(char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp < 0) {
        printf("Error filepath\n");
        return 1;
    }
    unsigned char c;
    while (fscanf(fp, "%c", &c) != EOF) {
        int x1 = 0, x2 = 0;
        if (c < 0) {
            x2 = (int) c;
            x2 = x2 & 0x000000ff;
            x2 = x2 << 8;
            x1 = x1 | x2;
            x2 = 0;
            fscanf(fp, "%c", &c);
            x2 = (int) c;
            x2 = x2 & 0x000000ff;
            x1 = x1 | x2; 
        } else {
            x1 = (int)c;
        }
        mp[x1] += 1;
        if (mp[x1] == 1)
            cnt ++;
    }
    map<int,int> :: iterator it;
    arr = (Node *) malloc (sizeof(Node) * (cnt + 5));
    int i = 0;
    for(it = mp.begin(); it != mp.end(); it++) {
        arr[i].ch = it->first;
        arr[i].p = it->second;
        arr[i].flag = 1;
        arr[i].lchild = arr[i].rchild = NULL;
        i++;
    }
    fclose(fp);
    return 0;
}

Node *getNewNode() {
    Node *p = (Node *) malloc (sizeof(Node));
    p->ch = 0;
    p->p = 0;
    p->flag = 0;
    p->lchild = p->rchild = NULL;
    return p;
}

HuffmanTree *getNewTree(int n) {
    HuffmanTree *t = (HuffmanTree*) malloc (sizeof(HuffmanTree));
    t->n = n;
    t->root = NULL;
    t->codes = (Code*) malloc (sizeof(Code) * n);
    return t;
}

int extractCodes(Node *root, Code *codes, int k, int l, char *buff) {
    buff[l] = 0;
    if (root->lchild == NULL && root->rchild == NULL) {
        codes[k].ch = root->ch;
        codes[k].str = strdup(buff);
        return 1;
    }
    int delta = 0;
    buff[l] = '0';
    delta += extractCodes(root->lchild, codes, k + delta, l + 1, buff);
    buff[l] = '1';
    delta += extractCodes(root->rchild, codes, k + delta, l + 1, buff);
    return delta;
}

void insertSort(Node **nodes, int n) {
    for (int j = n; j >= 1; j--) {
        if (nodes[j]->p > nodes[j - 1]->p) {
            swap(nodes[j], nodes[j - 1]);
            continue;
        }
        break;
    }
    return;
}

HuffmanTree *build(int n) {
    Node **nodes = (Node **) malloc (sizeof(Node*) * n);
    for (int i = 0; i < n; i++) {
        nodes[i] = (arr + i);
    }
    for (int i = 1; i < n; i++) {
        insertSort(nodes, i);
    }
    for (int i = n - 1; i >= 1; i--) {
        Node *p = getNewNode();
        p->lchild = nodes[i - 1];
        p->rchild = nodes[i];
        p->p = p->lchild->p + p->rchild->p;
        nodes[i - 1] = p;
        insertSort(nodes, i - 1);
    }
    char *buff = (char *) malloc (sizeof(char) * n);
    HuffmanTree *tree = getNewTree(n);
    tree->root = nodes[0];
    extractCodes(tree->root, tree->codes, 0, 0, buff);
    free(nodes);
    free(buff);
    return tree;
}





