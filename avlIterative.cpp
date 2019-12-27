#include<bits/stdc++.h>
using namespace std;

//-------------------- defined the Semaphore here ---------------------------
typedef struct __Zem_t {
    int value;
    pthread_cond_t cond;
    pthread_mutex_t lock;
}Zem_t;
void Zem_init(Zem_t *s, int value){
    s->value = value;
    s->cond = PTHREAD_COND_INITIALIZER;
    s->lock = PTHREAD_MUTEX_INITIALIZER;
}
void Zem_wait(Zem_t *s){
    pthread_mutex_lock(&(s->lock));
    while (s->value <= 0)
        pthread_cond_wait(&(s->cond), &(s->lock));
    s->value--;
    pthread_mutex_unlock(&(s->lock));
}
void Zem_post(Zem_t *s) {
    pthread_mutex_lock(&(s->lock));
    s->value++;
    pthread_cond_signal(&(s->cond));
    pthread_mutex_unlock(&(s->lock));
}
//-------------------------------------------------------------------------
// deifed read write lock here---------------------------------------------
typedef struct _rwlock_t{
    Zem_t lock;      
    Zem_t writelock; 
    int readers;   
}rwlock_t;

void rwlock_init(rwlock_t* rw){
    rw->readers = 0;
    Zem_init(&rw->lock, 1);
    Zem_init(&rw->writelock, 1);
}

void rwlock_acquire_readlock(rwlock_t*rw){ 
    Zem_wait(&rw->lock);
    rw->readers++;
    if(rw->readers == 1) 
        Zem_wait(&rw->writelock);
    Zem_post(&rw->lock);
}

void rwlock_release_readlock(rwlock_t*rw){
    Zem_wait(&rw->lock);
    rw->readers--;
    if(rw->readers == 0) 
        Zem_post(&rw->writelock);
    Zem_post(&rw->lock);
}

void rwlock_acquire_writelock(rwlock_t*rw){
    Zem_wait(&rw->writelock);
}

void rwlock_release_writelock(rwlock_t*rw){
    Zem_post(&rw->writelock);
}

//----------------------------------------------------------------
rwlock_t master_lock;

class _node{
    public:
        int data;
        int height;
        _node* left; _node* right;
        _node(int val){
            height = 1; data = val;
            left = NULL; right = NULL;
        }
};

class _avlTree{
    public:
        _node* root = NULL;

        void update(int x, int y){
	    
            _delete(x);
            insert(y);
	    
        }
        void inorder(){
            rwlock_acquire_readlock(&master_lock);
            _inorder(root);
            cout<<endl;
            rwlock_release_readlock(&master_lock);
        }
        void preorder(){
            rwlock_acquire_readlock(&master_lock);
            _preorder(root);
            cout<<endl;
            rwlock_release_readlock(&master_lock);
        }
        int height(_node* head){
            if(head == NULL) return 0;
            return head->height;
        }

        _node* rightRotation(_node* head){
            _node* newhead = head->left;
            head->left = newhead->right;
            newhead->right = head;
            head->height = 1+max(height(head->left), height(head->right));
            newhead->height = 1+max(height(newhead->left), height(newhead->right));
            return newhead;
        }

        _node* leftRotation(_node* head){
            _node* newhead = head->right;
            head->right = newhead->left;
            newhead->left = head;
            head->height = 1+max(height(head->left), height(head->right));
            newhead->height = 1+max(height(newhead->left), height(newhead->right));
            return newhead;
        }

        void insert(int x){
            rwlock_acquire_writelock(&master_lock);
            if(root == NULL){
                root = new _node(x);
                rwlock_release_writelock(&master_lock);
                return;
            }
            _node* head = root;
            stack<pair<_node*,char> > s;
            while(head != NULL){
                pair<_node*,char> p;
                p.first = head;
                if(x < head->data){
                    head = head->left;
                    p.second = 'l';
                }
                else if(x > head->data){
                    head = head->right;
                    p.second = 'r';
                }
                else if(x == head->data){
                    rwlock_release_writelock(&master_lock);
                    return;
                }
                s.push(p);
            }
            // here I have recieved a head that is null so parent ka agar left hai to left initiate else right initiate
            if(s.top().second == 'l') s.top().first->left = new _node(x);
            else if(s.top().second == 'r') s.top().first->right = new _node(x);
            // check in the stack for the balance to hold fine.
            while(!s.empty()){
                pair<_node*,char> temp = s.top();
                s.pop();
                temp.first->height = 1 + max(height(temp.first->left), height(temp.first->right));
                int bal = height(temp.first->left) - height(temp.first->right);
                if(bal > 1){
                    if(x < temp.first->left->data){
                        if(s.empty()) root = rightRotation(temp.first);
                        else{
                            if(s.top().second == 'l')
                                s.top().first->left = rightRotation(temp.first);
                            else if(s.top().second == 'r')
                                s.top().first->right = rightRotation(temp.first);
                        }
                    }else{
                        temp.first->left = leftRotation(temp.first->left);
                        if(s.empty()) root = rightRotation(temp.first);
                        else{
                            if(s.top().second == 'l')
                                s.top().first->left = rightRotation(temp.first);
                            else if(s.top().second == 'r')
                                s.top().first->right = rightRotation(temp.first);
                        }
                    }
                }
                else if(bal < -1){
                    if(x > temp.first->right->data){
                        if(s.empty()) root = leftRotation(temp.first);
                        else{
                            if(s.top().second == 'l')
                                s.top().first->left = leftRotation(temp.first);
                            else if(s.top().second == 'r')
                                s.top().first->right = leftRotation(temp.first);
                        }
                    }
                    else{
                        temp.first->right = rightRotation(temp.first->right);
                        if(s.empty()) root = leftRotation(temp.first);
                        else{
                            if(s.top().second == 'l')
                                s.top().first->left = leftRotation(temp.first);
                            else if(s.top().second == 'r')
                                s.top().first->right = leftRotation(temp.first);
                        }
                    }
                }
            }
            rwlock_release_writelock(&master_lock);
        }

        void _delete(int x){
            rwlock_acquire_writelock(&master_lock);
            if(root == NULL){
                rwlock_release_writelock(&master_lock);
                return;
            }
            _node* head = root;
            stack<pair<_node*,char> > s;
            while(head->data != x){
                pair<_node*,char> p;
                p.first = head;
                if(x < head->data){
                    head = head->left;
                    p.second = 'l';
                }
                else if(x > head->data){
                    head = head->right;
                    p.second = 'r';
                }
                s.push(p);
                if(head == NULL){
                    rwlock_release_writelock(&master_lock);
                    return;
                }
            }
            // I now have head->data = x; I want to delete this node.
            if(head->right == NULL && head->left == NULL){
                if(s.top().second == 'l') s.top().first->left = NULL;
                else if(s.top().second == 'r') s.top().first->right = NULL;
            }
            else if(head->left != NULL){
                //left ke rightmost se replace yha do scene hai ya to wo leaf hoga ya atmost one left child
                s.push(make_pair(head,'l'));
                _node* temp = head->left;
                while(temp->right != NULL){
                    s.push(make_pair(temp,'r'));
                    temp = temp->right;
                }
                if(temp->left != NULL){
                    head->data = temp->data;
                    temp = temp->left;
                }
                else{
                    head->data = temp->data;
                    if(s.top().second == 'l') s.top().first->left = NULL;
                    else if(s.top().second == 'r') s.top().first->right = NULL;
                }
            }
            else if(head->right != NULL){
                //right ke leftmost se replace yha do scene hai ya to wo leaf hoga ya atmost one right child
                s.push(make_pair(head,'r'));
                _node* temp = head->right;
                while(temp->left != NULL){
                    s.push(make_pair(temp,'l'));
                    temp = temp->left;
                }
                if(temp->right != NULL){
                    head->data = temp->data;
                    temp = temp->right;
                }
                else{
                    head->data = temp->data;
                    if(s.top().second == 'l') s.top().first->left = NULL;
                    else if(s.top().second == 'r') s.top().first->right = NULL;
                }
            }

            // check for balance in stack elements
            while(!s.empty()){
                pair<_node*,char> temp = s.top();
                s.pop();
                temp.first->height = 1 + max(height(temp.first->left), height(temp.first->right));
                int bal = height(temp.first->left) - height(temp.first->right);
                if(bal > 1){
                    if(x > temp.first->left->data){
                        if(s.empty()) root = rightRotation(temp.first);
                        else{
                            if(s.top().second == 'l')
                                s.top().first->left = rightRotation(temp.first);
                            else if(s.top().second == 'r')
                                s.top().first->right = rightRotation(temp.first);
                        }
                    }else{
                        temp.first->left = leftRotation(temp.first->left);
                        if(s.empty()) root = rightRotation(temp.first);
                        else{
                            if(s.top().second == 'l')
                                s.top().first->left = rightRotation(temp.first);
                            else if(s.top().second == 'r')
                                s.top().first->right = rightRotation(temp.first);
                        }
                    }
                }
                else if(bal < -1){
                    if(x < temp.first->right->data){
                        if(s.empty()) root = leftRotation(temp.first);
                        else{
                            if(s.top().second == 'l')
                                s.top().first->left = leftRotation(temp.first);
                            else if(s.top().second == 'r')
                                s.top().first->right = leftRotation(temp.first);
                        }
                    }
                    else{
                        temp.first->right = rightRotation(temp.first->right);
                        if(s.empty()) root = leftRotation(temp.first);
                        else{
                            if(s.top().second == 'l')
                                s.top().first->left = leftRotation(temp.first);
                            else if(s.top().second == 'r')
                                s.top().first->right = leftRotation(temp.first);
                        }
                    }
                }
            }
            rwlock_release_writelock(&master_lock);
        }

        void _inorder(_node* head){
            if(head == NULL) return;
            _inorder(head->left);
            cout<<head->data<<" ";
            _inorder(head->right);
        }
        
        void _preorder(_node* head){
            if(head == NULL) return;
            cout<<head->data<<" ";
            _preorder(head->left);
            _preorder(head->right);
        }
};

int main(){
    rwlock_init(&master_lock);
    _avlTree t;
    t.insert(13);
    t.insert(24);
    t.insert(35);
    t.insert(46);
    t.insert(57);
    t.insert(68);
    t.insert(79);
    t.inorder();
    t.update(46, 80);
    t.inorder();
    t._delete(46);
    t._delete(68);
    t._delete(79);
    t.inorder();
    t.insert(90); 
    t.inorder();
}







