/**************************************************************************
* Matrix implementation (split from Matrix.h for clarity).
*
* Piotr's Image&Video Toolbox      Version NEW
* Copyright 2008 Piotr Dollar.  [pdollar-at-caltech.edu]
* Please email me if you find bugs, or have suggestions or questions!
* Licensed under the Lesser GPL [see external/lgpl.txt]
**************************************************************************/
#ifndef _MATRIXIMPL_H
#define _MATRIXIMPL_H

template<class T1, class T2> void copy(Matrix<T1>& Mdest,const Matrix<T2>& Msrc)
{
	Mdest.setDims(Msrc.rows(), Msrc.cols());
	for(int i = 0; i < Msrc.rows(); i++)
		for(int j = 0; j < Msrc.cols(); j++)
			Mdest(i, j) = (T1)(Msrc(i, j));
}

template<class T> ostream& operator<<(ostream& os, const Matrix<T>& x)
{ //display matrix
	os << "[";
	for (int j=0; j<x.rows(); j++) {
		for (int i=0; i<x.cols(); i++) {
			os << x(j,i) << " ";
		}
		if( j!=x.rows()-1 )
			os << "\n";
	}
	os << "]";
	return os;
}

///////////////////////////////////////////////////////////////////////////////
template<class T>				Matrix<T>::Matrix()
{
	_mRows = 0;
	_nCols = 0;
	_data = NULL ;
	_dataIndex = NULL;
}

template<class T>				Matrix<T>::Matrix(int rows, int cols)
{
	_mRows = 0;
	_nCols = 0;
	_data = NULL ;
	_dataIndex = NULL;
	setDims(rows, cols);
}

template<class T>				Matrix<T>::Matrix(int rows, int cols, T value )
{
	_mRows = 0;
	_nCols = 0;
	_data = NULL ;
	_dataIndex = NULL;
	setDims(rows, cols, value);
}

template<class T>				Matrix<T>::Matrix(const Matrix& x)
{
	_data = NULL ;
	_dataIndex = NULL;
	_mRows = 0;
	_nCols = 0;
	setDims(x._mRows, x._nCols);
	for (int i = 0; i < size(); i++)
		(*this)(i) = x(i);
}

template<class T>				Matrix<T>::~Matrix()
{
	clear();
}

template<class T> void			Matrix<T>::clear()
{
	if (_data != NULL)
		delete[] _data;
	_data = NULL;
	if (_dataIndex != NULL)
		delete []_dataIndex;
	_dataIndex = NULL;
	_mRows = _nCols = 0;
}

template<class T> Matrix<T>&	Matrix<T>::operator= (const Matrix<T> &x )
{
	if ( this != &x ) {
		setDims( x.rows(), x.cols() );
		for (int i = 0; i < size(); i++)
			(*this)(i) = x(i);
	}
	return *this;
}

template<class T> Matrix<T>&	Matrix<T>::operator= (const vector<T> &x)
{
	setDims( x.size(), 1 );
	for(int i=0; i<rows(); i++)
		(*this)(i) = x[i];
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
template<class T> const char*	Matrix<T>::getCname() const
{
	static char cname[32];
	sprintf(cname,"Matrix<%s>",PRIMNAME(T));
	return cname;
}

template<class T> void			Matrix<T>::toObjImg( ObjImg &oi, const char *name ) const
{
	oi.init(name,getCname(),3);
	oi._children[0].frmPrim("mRows",&_mRows);
	oi._children[1].frmPrim("nCols",&_nCols);
	oi._children[2].frmPrim("data",_data,size());
}

template<class T> void			Matrix<T>::frmObjImg( const ObjImg &oi, const char *name )
{
	clear(); oi.check(name,getCname(),3,3);
	int mRows; oi._children[0].toPrim("mRows",&mRows);
	int nCols; oi._children[1].toPrim("nCols",&nCols);
	if(mRows && nCols) setDims(mRows,nCols); else return;
	oi._children[2].toPrim("data",_data);
}

template<class T> bool			Matrix<T>::writeToTxt( const char *fName, char *delim )
{
	remove( fName );
	ofstream strm; strm.open(fName, std::ios::out);
	if (strm.fail()) { abortError( "unable to write:", fName, __LINE__, __FILE__ ); return false; }
	for( int r=0; r<rows(); r++ ) {
		for( int c=0; c<cols(); c++ ) {
			strm << (*this)(r,c);
			if( c<(cols()-1)) strm << delim;
		}
		strm << endl;
	}
	strm.close();
	return true;
}

template<class T> bool			Matrix<T>::readFrmTxt( const char *fName, char *delim )
{
	ifstream strm; strm.open(fName, std::ios::in);
	if( strm.fail() ) return false;
	char * tline = new char[40000000];

	// get number of cols
	strm.getline( tline, 40000000 );
	int ncols = ( strtok(tline," ,")==NULL ) ? 0 : 1;
	while( strtok(NULL," ,")!=NULL ) ncols++;

	// read in each row
	strm.seekg( 0, ios::beg );
	Matrix<T> *rowVec; vector<Matrix<T>*> allRowVecs;
	while(!strm.eof() && strm.peek()>=0) {
		strm.getline( tline, 40000000 );
		rowVec = new Matrix<T>(1,ncols);
		(*rowVec)(0,0) = (T) atof( strtok(tline,delim) );
		for( int col=1; col<ncols; col++ )
			(*rowVec)(0,col) = (T) atof( strtok(NULL,delim) );
		allRowVecs.push_back( rowVec );
	}
	int mrows = allRowVecs.size();

	// finally create matrix
	setDims(mrows,ncols);
	for( int row=0; row<mrows; row++ ) {
		rowVec = allRowVecs[row];
		for( int col=0; col<ncols; col++ )
			(*this)(row,col) = (*rowVec)(0,col);
		delete rowVec;
	}
	allRowVecs.clear();
	delete [] tline;
	strm.close();
	return true;
}

///////////////////////////////////////////////////////////////////////////////
template<class T> bool			Matrix<T>::setDims(const int rows, const int cols)
{
	assert(rows>=0 && cols>=0 );
	if (rows==_mRows && cols==_nCols) return true;
	clear();  _mRows = rows;  _nCols = cols;
	if (_mRows==0 || _nCols==0) return true;

	int lSize = ((int)_mRows) * _nCols;
	try {
		_data = new T[lSize];
		_dataIndex = new T*[_mRows];
	} catch( bad_alloc& ) {
		cout << "Matrix::setDims(..)  OUT OF MEMORY" << endl;
		return false;
	}

	for (int i=0; i<_mRows; i++)
		_dataIndex[i] = &(_data[_nCols*i]);
	return true;
}

template<class T> bool			Matrix<T>::setDims(const int rows, const int cols, const T value )
{
	if( !setDims(rows,cols) )
		return false;
	setVal( value );
	return true;
}

template<class T> bool			Matrix<T>::resizeTo(const int rows1, const int cols1)
{
	if( rows1==rows() && cols1==cols() )
		return true;

	int lsize = (int)rows1*cols1;
	T *data1, **pdata1;
	try{
		data1 = new T[lsize];
		pdata1 = new T*[rows1];
	} catch( bad_alloc& ) {
		cout << "Matrix::resizeTo(..)  OUT OF MEMORY" << endl;
		return false;
	}

	for (int j=0; j<rows1; j++)
		pdata1[j] = &(data1[cols1*j]);
	for (int j=0; j<std::min((int)rows(),rows1); j++)
		for (int i=0; i<std::min((int)cols(),cols1); i++)
			pdata1[j][i] = (*this)(j, i);
	clear();
	_mRows = rows1;
	_nCols = cols1;
	_data = data1;
	_dataIndex = pdata1;
	return true;
}

template<class T> inline bool	Matrix<T>::valid(const int row, const int col) const
{
	return (row>=0 && row<_mRows && col>=0 && col<_nCols);
}

template<class T> inline bool	Matrix<T>::valid(const int index) const
{
	return (index>=0 && index<size());
}

template<class T> inline T&		Matrix<T>::operator() ( const int row, const int col)
{
	return _dataIndex[row][col];
}

template<class T> inline T&		Matrix<T>::operator() ( const int row, const int col) const
{
	return _dataIndex[row][col];
}

template<class T> inline T&		Matrix<T>::operator() ( const int index)
{
	return _data[index];
}

template<class T> inline T&		Matrix<T>::operator() ( const int index) const
{
	return _data[index];
}

///////////////////////////////////////////////////////////////////////////////
template<class T> Matrix<T>&	Matrix<T>::zero()
{
	if (_data != NULL)
		memset(_data, 0, sizeof(T)*_mRows*_nCols);
	return *this;
}

template<class T> Matrix<T>&	Matrix<T>::setVal(T value)
{
	if (_data != NULL) {
		if( value==0 ) zero(); else
			for (int i = 0; i < size(); i++)
				(*this)(i) = value;
	}
	return *this;
}

template<class T> Matrix<T>&	Matrix<T>::identity()
{
	for (int i = 0; i < rows();  i++)
		for (int j = 0; j < cols();  j++)
			(*this)(i,j) = (i==j) ? 1 : 0;
	return *this;
}

template<class T> Matrix<T>&	Matrix<T>::transpose()
{
	Matrix temp;
	temp.setDims(_nCols, _mRows);
	for (int i = 0; i < _nCols; i++)
		for (int j = 0; j < _mRows; j++)
			temp(i, j) = (*this)(j, i);
	(*this) = temp;
	return *this;
}

template<class T> Matrix<T>&	Matrix<T>::absolute()
{
	T z; z = 0;
	for (int i=0; i<size(); i++) {
		if ((*this)(i)<z)
			(*this)(i) = z-(*this)(i);
		else
			(*this)(i) = (*this)(i);
	}
	return (*this);
}

template<class T> void			Matrix<T>::rot90( Matrix<T> &B, int K) const
{
	int i,j;
	int k = K%4; if (k < 0) k += 4;
	if (k==1) {
		B.setDims(_nCols,_mRows);
		for( i=0; i < _nCols; i++) for( j=0; j < _mRows; j++)
			B(i, j) = (*this)(j, _nCols-i-1);
	} else if (k==2) {
		B.setDims(_mRows,_nCols);
		for( i=0; i < _nCols; i++) for( j=0; j < _mRows; j++)
			B(j, i) = (*this)(_mRows-j-1, _nCols-i-1);
	} else if (k==3) {
		B.setDims(_nCols,_mRows);
		for( i=0; i < _nCols; i++) for( j=0; j < _mRows; j++)
			B(i, j) = (*this)( _mRows-j-1, i);
	} else {
		B = *this;
	}
}

template<class T> void			Matrix<T>::fliplr( Matrix<T> &B) const
{
	int i,j;
	B.setDims(_nCols,_mRows);
	for( i=0; i<_mRows; i++ )
		for( j=0; j<_nCols; j++ )
			B(i,j) = (*this)(_mRows-i-1,j);
}

template<class T> void			Matrix<T>::flipud( Matrix<T> &B) const
{
	int i,j;
	B.setDims(_nCols,_mRows);
	for( i=0; i<_mRows; i++ )
		for( j=0; j<_nCols; j++ )
			B(i,j) = (*this)(i,_nCols-j-1);
}

template<class T> void			Matrix<T>::reshape( Matrix<T> &B, int mrows, int ncols ) const
{
	if( mrows*ncols != rows()*cols() ) {
		abortError( "cannot reshape, product of rows & cols has changed", __LINE__, __FILE__ );
		return;
	}
	B.setDims( mrows, ncols );
	for( int i=0; i<size(); i++ )
		B(i) = (*this)(i);
}

template<class T> T				Matrix<T>::prod() const
{
	T value; value = 1;
	for (int i=0; i<size(); i++)
		value *= (*this)(i);
	return value;
}

template<class T> T				Matrix<T>::sum() const
{
	T	value;
	value = 0;
	for (int i=0; i<size(); i++)
		value += (*this)(i);
	return value;
}

template<class T> T				Matrix<T>::trace() const
{
	assert(_mRows==_nCols);
	T d; d=0;
	for( int j=0; j<_mRows; j++)
		d += (*this)(j,j);
	return d;
}

template<class T> T				Matrix<T>::min() const
{
	T value=0;
	if (size()>0) {
		int i;
		value = (*this)(0);
		for (i=1; i<size(); i++)
			if ((*this)(i)<value)
				value = (*this)(i);
	}
	return value;
}

template<class T> int			Matrix<T>::mini() const
{
	int ind=0;
	if (size()>0) {
		T value = (*this)(0);
		for( int i=1; i<size(); i++)
			if ((*this)(i)<value ){
				ind=i;
				value = (*this)(i);
			}
	}
	return ind;
}

template<class T> T				Matrix<T>::max() const
{
	T value=0;
	if (size()>0) {
		int i;
		value = (*this)(0);
		for (i=1; i<size(); i++)
			if ((*this)(i)>value)
				value = (*this)(i);
	}
	return value;
}

template<class T> int			Matrix<T>::maxi() const
{
	int ind=0;
	if (size()>0) {
		int i;
		T value = (*this)(0);
		for (i=1; i<size(); i++)
			if ((*this)(i)>value){
				ind=i;
				value = (*this)(i);
			}
	}
	return ind;
}

///////////////////////////////////////////////////////////////////////////////
template<class T> Matrix<T>		Matrix<T>::operator* ( const Matrix<T> &b ) const
{  //multiply two matrices
	T			sum;
	Matrix<T>	temp;
	int nrow=rows(),ncol=b.cols(),xncol=cols();
	temp.setDims(nrow,ncol);
	for (int i = 0; i < nrow; i++)
		for (int j = 0; j < ncol; j++) {
			sum = 0;
			for(int k = 0; k < xncol; k++)
				sum = sum+(*this)(i, k)*b(k, j); //multiply
			temp(i, j) = sum;
		}
		return temp;
}

template<class T> Matrix<T>		Matrix<T>::operator& ( const Matrix<T> &b ) const
{
	assert(rows()==b.rows() && cols()==b.cols());
	Matrix<T> temp;
	temp.setDims(rows(),cols());
	for (int i=0; i<size(); i++)
		temp(i) = (*this)(i)*b(i);
	return temp;
}

template<class T> Matrix<T>		Matrix<T>::operator^ ( const Matrix<T> &b ) const
{
	Matrix<T>	temp(rows(),cols());
	int i, n=size();
	for (i=0; i<n; i++)
		temp(i) = pow((*this)(i),b(i));
	return temp;
}

template<class T> Matrix<T>		Matrix<T>::operator^ ( const T &b ) const
{
	Matrix<T> temp(rows(),cols());
	int i, n=size();
	for (i=0; i<n; i++)
		temp(i) = pow((*this)(i),b);
	return temp;
}

template<class T> Matrix<T>		operator^ ( const T a, const Matrix<T> &b )
{
	Matrix<T> temp(b.rows(),b.cols());
	int i, n=b.size();
	for (i=0; i<n; i++)
		temp(i) = pow(a,b(i));
	return temp;
}

// Pointwise (Matrix OP Matrix) operations - except *,&,^
#define DOP(OP) \
	template<class T> Matrix<T> Matrix<T>::operator OP ( const Matrix<T> &b ) const { \
	Matrix<T> c (rows(), cols()); \
	for (int i = 0; i < size(); i++) { \
	c(i) = (*this)(i) OP b(i); \
	} \
	return c; \
}
DOP(+); DOP(-); DOP(/); DOP(<); DOP(>); DOP(<=); DOP(>=); DOP(&&); DOP(||); DOP(==); DOP(!=);
#undef DOP
// Pointwise (Matrix<T> OP T) and (T OP Matrix<T>) operations - except ^
#define DOP(OP) \
	template<class T> Matrix<T> Matrix<T>::operator OP (const T &b ) const { \
	Matrix<T> c (rows(), cols()); \
	for (int i = 0; i < size(); i++) { \
	c(i) = (*this)(i) OP b; \
	} \
	return c; \
} \
	template<class T> Matrix<T> operator OP (const T &a, const Matrix<T> &b ) { \
	Matrix<T> c (b.rows(), b.cols()); \
	for (int i = 0; i < b.size(); i++) { \
	c(i) = a OP b(i); \
	} \
	return c; \
}
DOP(+); DOP(-); DOP(/); DOP(*); DOP(<); DOP(>); DOP(<=); DOP(>=); DOP(&&); DOP(||); DOP(==); DOP(!=);
#undef DOP
// Pointwise assignment operators
#define DOP(OP) \
	template<class T> Matrix<T>& Matrix<T>::operator OP (const T b ) { \
	for (int i = 0; i < size(); i++) \
	(*this)(i) OP b; \
	return *this; \
} \
	template<class T> Matrix<T>& Matrix<T>::operator OP (const Matrix<T> &b ) { \
	for (int i = 0; i < size(); i++) \
	(*this)(i) OP b(i); \
	return *this; \
}
DOP(+=); DOP(-=); DOP(*=); DOP(/=);
#undef DOP
#endif
