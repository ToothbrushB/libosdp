use std::{
    time::Duration,
    thread,
    result::Result,
};
use libosdp::{
    cp::ControlPanel,
    pdinfo::{PdInfo, OsdpFlag},
    channel::{OsdpChannel, unix_channel::UnixChannel},
    error::OsdpError, pdid::PdId,
};

fn main() -> Result<(), OsdpError> {
    env_logger::builder()
        .filter_level(log::LevelFilter::Info)
        .format_target(false)
        .format_timestamp(None)
        .init();
    let stream = UnixChannel::connect("conn-1")?;
    let pd_info = vec![
        PdInfo::new(
            "PD 101", 101,
            115200,
            OsdpFlag::EnforceSecure,
            PdId {
                version: 0x01,
                model: 0x02,
                vendor_code: 0x03,
                serial_number: 0x04,
                firmware_version: 0x05,
            },
            vec![],
            OsdpChannel::new::<UnixChannel>(Box::new(stream)),
            [
                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
            ]
        ),
    ];
    let mut cp = ControlPanel::new(pd_info)?;
    loop {
        cp.refresh();
        thread::sleep(Duration::from_millis(50));
    }
}