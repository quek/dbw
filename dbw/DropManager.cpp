#include "DropManager.h"
#include "App.h"
#include "util.h"

DropManager::DropManager(App* app) : _app(app)
{
}

HRESULT DropManager::QueryInterface(REFIID riid, void** ppvObject)
{
    if (riid == IID_IDropTarget)
    {
        *ppvObject = this;	// or static_cast<IUnknown*> if preferred
        // AddRef() if doing things properly
                    // but then you should probably handle IID_IUnknown as well;
        return S_OK;
    }

    *ppvObject = NULL;
    return E_NOINTERFACE;
}

HRESULT DropManager::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    // TODO: check whether we can handle this type of object at all and set *pdwEffect &= DROPEFFECT_NONE if not;

    // do something useful to flag to our application that files have been dragged from the OS into our application
    // ...

    // trigger MouseDown for button 1 within ImGui
    // ...

    *pdwEffect = DROPEFFECT_NONE; // WAVファイル以外の場合の処理

    FORMATETC fmtetc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stgmed;

    // IDataObjectからCF_HDROP形式のデータを取得しようと試みる
    if (SUCCEEDED(pDataObj->GetData(&fmtetc, &stgmed)))
    {
        // データが取得できた場合、HDROPハンドルを取得
        HDROP hDrop = (HDROP)GlobalLock(stgmed.hGlobal);
        if (hDrop)
        {
            UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0); // ドロップされたファイルの数
            std::vector<std::string> files;

            for (UINT i = 0; i < fileCount; i++)
            {
                TCHAR filePath[MAX_PATH];
                if (DragQueryFile(hDrop, i, filePath, MAX_PATH))
                {
                    std::wstring wFilePath = filePath;
                    std::string file = WideStringToAnsiString(wFilePath);
                    if (file.ends_with(".wav"))
                    {
                        files.push_back(file);
                    }
                }
            }
            if (!files.empty())
            {
                *pdwEffect = DROPEFFECT_COPY;
                _app->dragEnter(files);
            }

        }
        GlobalUnlock(stgmed.hGlobal);
        ReleaseStgMedium(&stgmed);
    }

    return S_OK;
}

HRESULT DropManager::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    *pdwEffect &= DROPEFFECT_COPY;
    return S_OK;
}

HRESULT DropManager::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    _app->drop();
    *pdwEffect &= DROPEFFECT_COPY;
    return S_OK;
}
