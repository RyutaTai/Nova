#pragma once

//  ポインタがnullptrでないことを確認し、deleteを実行した後にポインタをnullptrに設定する
template <typename T>
void SafeDelete(T*& p) //   ポインタへの参照を取ることで、呼び出し元のポインタもnullptrにできる
{
    if (p != nullptr)
    {
        delete p;
        p = nullptr;    //  削除後にポインタをnullptrに設定
    }
}

//  配列用のSafeDelete
//  配列を解放し、ポインタをnullptrに設定します。
template <typename T>
void SafeDeleteArray(T*& p)
{
    if (p != nullptr)
    {
        delete[] p;     //  ※配列のdeleteはdelete[]
        p = nullptr;
    }
}