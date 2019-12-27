#include<iostream>
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
    s->lock = PTHREAD_MUTEX_INITIALIZER;;
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

        void insert(int x){
            rwlock_acquire_writelock(&master_lock);
            root = _insert(root, x);
            rwlock_release_writelock(&master_lock);
        }
        void remove(int x){
            rwlock_acquire_writelock(&master_lock);
            root = _delete(root, x);
            rwlock_release_writelock(&master_lock);
        }
        void update(int x, int y){
            rwlock_acquire_writelock(&master_lock);
            root = _delete(root, x);
            root = _insert(root, y);
            rwlock_release_writelock(&master_lock);
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

        _node* _insert(_node* head, int x){
            if(head == NULL){
                _node* temp = new _node(x);
                return temp;
            }

            if(x < head->data) head->left = _insert(head->left, x);
            else if(x > head->data) head->right = _insert(head->right, x);

            head->height = 1 + max(height(head->left), height(head->right));
            
            int bal = height(head->left) - height(head->right);
            if(bal > 1){
                if(x < head->left->data){
                    return rightRotation(head);
                }
                else{
                    head->left = leftRotation(head->left);
                    return rightRotation(head);
                }
            }
            else if(bal < -1){
                if(x > head->right->data){
                    return leftRotation(head);
                }
                else{
                    head->right = rightRotation(head->right);
                    return leftRotation(head);
                }
            }
            return head;
        }

        _node* _delete(_node* head, int x){
            if(head==NULL) return NULL;
            
            if(x < head->data){
                head->left = _delete(head->left, x);
            }else if(x > head->data){
                head->right = _delete(head->right, x);
            }else{
                _node* r = head->right;
                if(head->right==NULL){
                    _node* l = head->left;
                    delete(head);
                    head = l;
                }else if(head->left==NULL){
                    delete(head);
                    head = r;
                }else{
                    while(r->left!=NULL) r = r->left;
                    head->data = r->data;
                    head->right = _delete(head->right, r->data);
                }
            }

            if(head == NULL) return head;
            head->height = 1 + max(height(head->left), height(head->right));
            
            int bal = height(head->left) - height(head->right);
            if(bal > 1){
                if(x > head->left->data){
                    return rightRotation(head);
                }
                else{
                    head->left = leftRotation(head->left);
                    return rightRotation(head);
                }
            }
            else if(bal < -1){
                if(x < head->right->data){
                    return leftRotation(head);
                }
                else{
                    head->right = rightRotation(head->right);
                    return leftRotation(head);
                }
            }
            return head;
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
    t.remove(57);
    t.remove(68);
    t.remove(79);
    t.inorder();
}
