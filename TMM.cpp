// TMM.cpp : Defines the entry point for the application.
//

#include "TMM.h"
#include "UI/ModWindow.h"
#include "UI/RootDirWindow.h"

#include <wx/stdpaths.h>
#include <wx/fileconf.h>

wxIMPLEMENT_APP(TMM);
  

const char* ConfigFile = "Settings.ini";
const char* ConfigFileRootKey = "RootDir";
const char* ConfigFileWaitForTeraKey = "WaitForTera";

const char* GameConfigFilePath = "ModList.tmm";

const char* CompositeMapperFile = "CompositePackageMapper.dat";
const char* CompositeMapperBackupFile = "CompositePackageMapper.clean";

const char* CookedPcDir = "CookedPC";
const char* ModsStorageDir = "CookedPC";


wxString GetConfigPath()
{
  wxString path = wxStandardPaths::Get().GetUserLocalDataDir() + wxFILE_SEP_PATH;
  if (!wxDirExists(path))
  {
    wxMkDir(path);
  }
  path += ConfigFile;
  return path;
}

void TMM::LoadAppConfig()
{
  wxString path = GetConfigPath();
  wxFileConfig cfg(wxEmptyString, wxEmptyString, path);
  cfg.SetPath(path);

  RootDir = cfg.Read(ConfigFileRootKey, wxEmptyString).ToStdWstring();
  WaitForTera = cfg.ReadBool(ConfigFileWaitForTeraKey, false);
}

void TMM::SaveAppConfig()
{
  wxString path = GetConfigPath();
  wxFileConfig cfg(wxEmptyString, wxEmptyString, path);
  cfg.SetPath(path);

  wxString tmpPath = RootDir.wstring();
  cfg.Write(ConfigFileRootKey, tmpPath);
  cfg.Write(ConfigFileWaitForTeraKey, WaitForTera);
}

void TMM::LoadGameConfig()
{
  if (!std::filesystem::exists(GameConfigPath))
  {
    SaveGameConfig();
  }
  std::ifstream s(GameConfigPath, std::ios::binary | std::ios::in);
  s >> GameConfig;
}

void TMM::SaveGameConfig()
{
  std::ofstream s(GameConfigPath, std::ios::binary | std::ios::out);
  s << GameConfig;
}

void TMM::ChangeRootDir(const std::filesystem::path& newRootDir)
{
  RootDir = newRootDir;
  if (!SetupPaths())
  {
    ExitMainLoop();
    return;
  }
  SaveAppConfig();
  GameConfig.Mods.clear();
  LoadGameConfig();
  ModWindow* mainWindow = new ModWindow(nullptr, GameConfig.Mods);
  mainWindow->Show();
}

void TMM::UpdateModsList(const std::vector<ModEntry> modData)
{
  GameConfig.Mods = modData;
  SaveGameConfig();
}

bool TMM::BackupCompositeMapperFile()
{
    if (!std::filesystem::exists(CompositeMapperPath))
    {
        return false;
    }

    if (std::filesystem::exists(BackupCompositeMapperPath))
    {
        return true;
    }

    std::error_code err;
    std::filesystem::copy_file(CompositeMapperPath, BackupCompositeMapperPath, err);

    if (err)
    {
        return false;
    }

    return true;
}

int TMM::OnRun()
{
  LoadAppConfig();

  if (!SetupPaths())
  {
    return 0;
  }

  SaveAppConfig();

  LoadGameConfig();
  ModWindow* mainWindow = new ModWindow(nullptr, GameConfig.Mods);
  mainWindow->Show();
  return wxApp::OnRun();
}

bool TMM::SetupPaths()
{
    if (RootDir.empty() || !std::filesystem::exists(RootDir))
    {
        RootDirWindow rooWin(nullptr, RootDir.wstring());
        if (rooWin.ShowModal() == wxID_OK)
        {
            RootDir = rooWin.GetPath().ToStdWstring();
        }
        if (RootDir.empty() || !std::filesystem::exists(RootDir))
        {
            return false;
        }
    }

    CompositeMapperPath = RootDir / CookedPcDir / CompositeMapperFile;
    BackupCompositeMapperPath = (RootDir / ModsStorageDir) / CompositeMapperBackupFile;

    if (!BackupCompositeMapperFile())
    {
        wxMessageBox(
            _("Could not create the backup file \"CompositePackageMapper.clean\".\n\n"
                "TMM needs to make a one-time backup of the original CompositePackageMapper.dat.\n"
                "Please ensure:\n"
                "TERA is not running\n"
                "You have write permissions in the CookedPC folder\n"
                "Sufficient disk space is available\n\n"
                "After fixing the issue, restart TMM."),
            _("Backup Creation Failed"),
            wxICON_ERROR
        );
        return false;
    }

    while (!std::filesystem::exists(CompositeMapperPath))
    {
        wxMessageBox(_("Couldn't find \"S1Game\\CookedPC\\CompositePackageMapper.dat\" file."), _("Error!"), wxICON_ERROR);
        RootDirWindow rooWin(nullptr, RootDir.wstring());
        if (rooWin.ShowModal() != wxID_OK)
        {
            return false;
        }
        RootDir = rooWin.GetPath().ToStdWstring();
        if (RootDir.empty() || !std::filesystem::exists(RootDir))
        {
            return false;
        }
        CompositeMapperPath = RootDir / CookedPcDir / CompositeMapperFile;
        BackupCompositeMapperPath = (RootDir / ModsStorageDir) / CompositeMapperBackupFile;

        if (!BackupCompositeMapperFile())
        {
            wxMessageBox(
                _("Failed to create backup after changing directory.\n"
                    "Please ensure write permissions and that TERA is not running."),
                _("Error!"),
                wxICON_ERROR
            );
            return false;
        }
    }

    ClientDir = RootDir.parent_path();
    ModsDir = RootDir / ModsStorageDir;
    GameConfigPath = ModsDir / GameConfigFilePath;

    return true;
}