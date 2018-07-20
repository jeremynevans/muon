// Copyright 2016 Brave Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "brave/browser/component_updater/brave_component_updater_configurator.h"

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "base/command_line.h"
#include "base/strings/sys_string_conversions.h"
#include "base/version.h"
#if defined(OS_WIN)
#include "base/win/win_util.h"
#include "chrome/install_static/install_util.h"
#endif
#include "chrome/browser/browser_process.h"
#include "chrome/browser/net/system_network_context_manager.h"
#include "components/component_updater/component_updater_command_line_config_policy.h"
#include "components/component_updater/configurator_impl.h"
#include "components/prefs/pref_service.h"
#include "components/update_client/component_patcher_operation.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/service_manager_connection.h"
#include "net/url_request/url_request_context_getter.h"


namespace component_updater {

namespace {

class BraveConfigurator : public update_client::Configurator {
 public:
  BraveConfigurator(const base::CommandLine* cmdline,
                    net::URLRequestContextGetter* url_request_getter,
                    bool use_brave_server);

  // update_client::Configurator overrides.
  int InitialDelay() const override;
  int NextCheckDelay() const override;
  int OnDemandDelay() const override;
  int UpdateDelay() const override;
  std::vector<GURL> UpdateUrl() const override;
  std::vector<GURL> PingUrl() const override;
  std::string GetProdId() const override;
  base::Version GetBrowserVersion() const override;
  std::string GetChannel() const override;
  std::string GetBrand() const override;
  std::string GetLang() const override;
  std::string GetOSLongName() const override;
  std::string ExtraRequestParams() const override;
  std::string GetDownloadPreference() const override;
  scoped_refptr<net::URLRequestContextGetter> RequestContext() const override;
  scoped_refptr<network::SharedURLLoaderFactory> URLLoaderFactory()
      const override;
  std::unique_ptr<service_manager::Connector> CreateServiceManagerConnector()
      const override;
  bool EnabledDeltas() const override;
  bool EnabledComponentUpdates() const override;
  bool EnabledBackgroundDownloader() const override;
  bool EnabledCupSigning() const override;
  PrefService* GetPrefService() const override;
  update_client::ActivityDataService* GetActivityDataService() const override;
  bool IsPerUserInstall() const override;
  std::vector<uint8_t> GetRunActionKeyHash() const override;
  std::string GetAppGuid() const override;

 private:
  friend class base::RefCountedThreadSafe<BraveConfigurator>;

  ConfiguratorImpl configurator_impl_;
  bool use_brave_server_;

  ~BraveConfigurator() override {}
};

// Allows the component updater to use non-encrypted communication with the
// update backend. The security of the update checks is enforced using
// a custom message signing protocol and it does not depend on using HTTPS.
BraveConfigurator::BraveConfigurator(
    const base::CommandLine* cmdline,
    net::URLRequestContextGetter* url_request_getter,
    bool use_brave_server)
    : configurator_impl_(ComponentUpdaterCommandLineConfigPolicy(cmdline),
                         false),
      use_brave_server_(use_brave_server) {}

int BraveConfigurator::InitialDelay() const {
  return configurator_impl_.InitialDelay();
}

int BraveConfigurator::NextCheckDelay() const {
  return configurator_impl_.NextCheckDelay();
}

int BraveConfigurator::OnDemandDelay() const {
  return configurator_impl_.OnDemandDelay();
}

int BraveConfigurator::UpdateDelay() const {
  return configurator_impl_.UpdateDelay();
}

std::vector<GURL> BraveConfigurator::UpdateUrl() const {
  if (use_brave_server_) {
    // For localhost of vault-updater
    // return std::vector<GURL> {GURL("http://localhost:8192/extensions")};
    return std::vector<GURL>
        {GURL("https://laptop-updates.brave.com/extensions")};
  }

  // For Chrome's component store
  return configurator_impl_.UpdateUrl();
}

std::vector<GURL> BraveConfigurator::PingUrl() const {
  return UpdateUrl();
}

std::string BraveConfigurator::GetProdId() const {
  return std::string();
}

base::Version BraveConfigurator::GetBrowserVersion() const {
  return configurator_impl_.GetBrowserVersion();
}

std::string BraveConfigurator::GetChannel() const {
  return std::string("stable");
}

std::string BraveConfigurator::GetBrand() const {
  return std::string();
}

std::string BraveConfigurator::GetLang() const {
  return std::string();
}

std::string BraveConfigurator::GetOSLongName() const {
  return configurator_impl_.GetOSLongName();
}

std::string BraveConfigurator::ExtraRequestParams() const {
  return configurator_impl_.ExtraRequestParams();
}

std::string BraveConfigurator::GetDownloadPreference() const {
  return std::string();
}

scoped_refptr<net::URLRequestContextGetter>
  BraveConfigurator::RequestContext() const {
    return g_browser_process->system_request_context();
}

scoped_refptr<network::SharedURLLoaderFactory>
BraveConfigurator::URLLoaderFactory() const {
  SystemNetworkContextManager* system_network_context_manager =
      g_browser_process->system_network_context_manager();
  // Manager will be null if called from InitializeForTesting.
  if (!system_network_context_manager)
    return nullptr;
  return system_network_context_manager->GetSharedURLLoaderFactory();
}

std::unique_ptr<service_manager::Connector>
    BraveConfigurator::CreateServiceManagerConnector() const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return content::ServiceManagerConnection::GetForProcess()
      ->GetConnector()
      ->Clone();
}

bool BraveConfigurator::EnabledComponentUpdates() const {
  return configurator_impl_.EnabledComponentUpdates();
}

bool BraveConfigurator::EnabledDeltas() const {
  // TODO(bbondy): Re-enable
  // return configurator_impl_.DeltasEnabled();
  return false;
}

bool BraveConfigurator::EnabledBackgroundDownloader() const {
  return configurator_impl_.EnabledBackgroundDownloader();
}

bool BraveConfigurator::EnabledCupSigning() const {
  if (use_brave_server_) {
    return false;
  }
  return configurator_impl_.EnabledCupSigning();
}

PrefService* BraveConfigurator::GetPrefService() const {
  return nullptr;
}

update_client::ActivityDataService* BraveConfigurator::GetActivityDataService()
    const {
  return nullptr;
}

bool BraveConfigurator::IsPerUserInstall() const {
  return false;
}

std::vector<uint8_t> BraveConfigurator::GetRunActionKeyHash() const {
  return configurator_impl_.GetRunActionKeyHash();
}

std::string BraveConfigurator::GetAppGuid() const {
#if defined(OS_WIN)
  return install_static::UTF16ToUTF8(install_static::GetAppGuid());
#else
  return configurator_impl_.GetAppGuid();
#endif
}


}  // namespace

scoped_refptr<update_client::Configurator>
MakeBraveComponentUpdaterConfigurator(
    const base::CommandLine* cmdline,
    net::URLRequestContextGetter* context_getter,
    bool use_brave_server) {
  return new BraveConfigurator(cmdline, context_getter, use_brave_server);
}

}  // namespace component_updater
