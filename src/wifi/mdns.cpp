
#include <mdns.h>

#include "../app_params/app_params.h"
#include "../board.h"
#include "../version.h"
#include "wifi_loggy.h"
#include "wifi_utils.h"

void add_mdns_services() {
  // webserver service

  mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);

  static String serviceName = String(getOurHostname()) + " webconfig";
  mdns_service_instance_name_set("_http", "_tcp", serviceName.c_str());

  // slaesh service

  mdns_service_add(NULL, "_slaesh", "_tcp", 5788, NULL, 0);

  const auto friendlyName =
      iizi_get_parameter_value(IIZI_PARAM_FRIENDLY_NAME_KEY);

  mdns_txt_item_t slaeshTxtData[] = {
    {"dev-type", "inwall-relay"},

#if BOARD == BOARD_IWR_OLD_WO_INPUT
    {"hw", "old-wo-input"},
#else
    {"hw", "rev1"},
#endif

    {"sw", version},

    {"friendlyname", friendlyName},
  };

  mdns_service_txt_set("_slaesh", "_tcp", slaeshTxtData,
                       sizeof(slaeshTxtData) / sizeof(mdns_txt_item_t));
}

void mdns_update_services() {
  mdns_service_remove_all();

  add_mdns_services();
}

void start_mdns_service() {
  esp_err_t err = mdns_init();
  if (err) {
    wifiLoggy.printf("MDNS Init failed: %d\n", err);
    return;
  }

  const auto hostname = getOurHostname();

  // set hostname
  mdns_hostname_set(hostname);
  // set default instance
  mdns_instance_name_set(hostname);

  mdns_update_services();
}
