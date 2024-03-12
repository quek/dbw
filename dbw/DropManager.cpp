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
                        *pdwEffect = DROPEFFECT_COPY; // WAVファイルが見つかった場合の処理
                        files.emplace_back(file);
                    }
                }
            }
            if (!files.empty())
            {
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
    // trigger MouseMove within ImGui, position is within pt.x and pt.y
    // grfKeyState contains flags for control, alt, shift etc
    //;TODO ...

    *pdwEffect &= DROPEFFECT_COPY;
    return S_OK;
}

HRESULT DropManager::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    // grfKeyState contains flags for control, alt, shift etc

    // render the data into stgm using the data description in fmte
    FORMATETC fmte = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stgm;

    if (SUCCEEDED(pDataObj->GetData(&fmte, &stgm)))
    {
        HDROP hdrop = (HDROP)stgm.hGlobal; // or reinterpret_cast<HDROP> if preferred
        UINT file_count = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
        std::vector<std::string> files(file_count);

        // we can drag more than one file at the same time, so we have to loop here
        for (UINT i = 0; i < file_count; i++)
        {
            TCHAR szFile[MAX_PATH];
            UINT cch = DragQueryFile(hdrop, i, szFile, MAX_PATH);
            if (cch > 0 && cch < MAX_PATH)
            {
                // szFile contains the full path to the file, do something useful with it
                // i.e. add it to a vector or something
                std::wstring wFilePath = szFile;
                std::string file = WideStringToAnsiString(wFilePath);
                files.emplace_back(file);
            }
        }

        // we have to release the data when we're done with it
        ReleaseStgMedium(&stgm);

        // notify our application somehow that we've finished dragging the files (provide the data somehow)
        //;TODO ...
        _app->drop(files);
    }

    // trigger MouseUp for button 1 within ImGui
    //;TODO ...

    *pdwEffect &= DROPEFFECT_COPY;
    return S_OK;
}
