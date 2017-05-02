#include "Python.h"
extern void serial_init();
extern void read_sbus();
extern void serial_close();
extern void write_sbus();

PyObject* sbus_serial_init(PyObject* self, PyObject* args){
  int fd;

  //if (!PyArg_ParseTuple(args, "ii", &x, &y)) return NULL;
  serial_init();
  //return Py_BuildValue("i", 1);
  return Py_BuildValue("s", NULL);
}

#if 0
PyObject* sbus_read_sbus(PyObject* self, PyObject* args){

  //if (!PyArg_ParseTuple(args, "ii", &x, &y)) return NULL;
  read_sbus();
  //return Py_BuildValue("i", fd);
  return Py_BuildValue("i", 1);
}
#endif

PyObject* sbus_print_sbus(PyObject* self, PyObject* args){
  PyListObject *binary;
  binary = (PyListObject *) PyList_New(8);
  int b[8];
  //if (!PyArg_ParseTuple(args, "ii", &x, &y)) return NULL;
  //print_sbus(&b);
  read_sbus(&b);
  //return Py_BuildValue("i", fd);
  //return Py_BuildValue("i", 1);

  int i;
  int n=0;

  for (i=0; i<8; i++){
  //gray に1bitずつint型の値を入れていく
    PyList_SET_ITEM(binary, n++, Py_BuildValue("i", b[i]));
  }
   return Py_BuildValue("O", binary);
}

PyObject* sbus_serial_close(PyObject* self, PyObject* args){
   serial_close();
   return Py_BuildValue("s", NULL);
}


PyObject* sbus_write(PyObject* self, PyObject* args){
  const int MAX_BIT_LENGTH = 8;
  unsigned int num;
  unsigned int mask;
  int m,n,i,len;
  int b[MAX_BIT_LENGTH], inputed_binary[MAX_BIT_LENGTH];
  PyListObject *binary; //pythonのlistオブジェクトを表現
  PyObject *get_list;
  binary = (PyListObject *) PyList_New(MAX_BIT_LENGTH); 

  if (!PyArg_ParseTuple(args, "O", &get_list )){
    return NULL;
  }
  if PyList_Check(get_list) {
      for (i=0; i<PyList_Size(get_list); i++){
    //リストオブジェクトの中身をCで見れるように変換しながら取り出す?(自信なし)
    inputed_binary[i] = PyInt_AsSsize_t(PyList_GetItem(get_list, (Py_ssize_t)i)); //ok
      }
    }

  write_sbus(inputed_binary);

   return Py_BuildValue("s", NULL);
}

static PyMethodDef sbusmethods[] = {
  {"init", sbus_serial_init, METH_VARARGS},
  //{"read", sbus_read_sbus, METH_VARARGS },
  {"read", sbus_print_sbus, METH_VARARGS },
  {"close", sbus_serial_close, METH_VARARGS },
  {"write", sbus_write, METH_VARARGS },
  {NULL},
};

void initsbus()
{
  Py_InitModule("sbus", sbusmethods);
}
