inlets = 4;
outlets = 12;

var yaw = null;
var pitch = null;
var roll = null;
var flavour = null;

function deg2rad(deg){
	return deg * Math.PI / 180.0;
}

var ensureFloat = 0.00000000000001; // prevents Max making a float an int which it does automatically if its a whole number

var i = {
    "yaw_z_deg":   0,
    "pitch_y_deg": 1,
    "roll_x_deg":  2,
    "osc_flavour": 3

};


var o = {
    "debug":       0,
    "yaw_z_rad":   1,
    "pitch_y_rad": 2,
    "roll_x_rad":  3,
    "quat_w":      4,
    "quat_x":      5,
    "quat_y":      6,  
    "quat_z":      7,
	"req_update":  8,
	"osc_msg":     9,
	"osc_port":    10,
	"osc_flavour": 11
};

var flavours = [
	"ambix_quaternion",
	"ambix_rotation_float",
	"ambix_rotation_int",
	"ambix_head_pose_float",
	"ambix_head_pose_int",
	"iem",
	"unity",
	"mach1",
	"audiolab",
	"sparta_ypr",
	"sparta_euler_individual",
	"sparta_quaternion",
	"3d_tune-in",
	"hedrot"
]


function describe_in(num)
{
    for(desc in i){
        if(i[desc] === num){
            assist(desc, num);
            return;
        }
    }
    assist("[UNKNOWN]", num);
}
setinletassist(-1, describe_in);

function describe_out(num)
{
    for(desc in o){
        if(o[desc] === num){
            assist(desc, num);
            return;
        }
    }
    assist("[UNKNOWN]", num);
}
setoutletassist(-1, describe_out);

function msg_float(f){
	if(inlet === i.yaw_z_deg) yaw = f;
	if(inlet === i.pitch_y_deg) pitch = f;
	if(inlet === i.roll_x_deg) roll = f;
	if(inlet === i.osc_flavour) {
		if(f >= 0 && f < flavours.length){
	    	flavour = flavours[f];
		}
		outlet(o.osc_flavour, flavour);
		if(flavour.substr(0,6) === "ambix_") outlet(o.osc_port, 7120);
		if(flavour === "mach1") outlet(o.osc_port, 9898);
		if(flavour === "audiolab") outlet(o.osc_port, 9000);
		if(flavour.substr(0,7) === "sparta_") outlet(o.osc_port, 9000);
		if(flavour === "hedrot") outlet(o.osc_port, 2001);
		if(flavour === "3d_tune-in") outlet(o.osc_port, 12300);
	}
	process();
}

function process()
{

	if(yaw === null) return;
	if(pitch === null) return;
	if(roll === null) return;
	if(flavour === null) return;
	
	var yawrad = deg2rad(yaw);
	var pitchrad = deg2rad(pitch);
	var rollrad = deg2rad(roll);
	outlet(o.yaw_z_rad, yawrad);
	outlet(o.pitch_y_rad, pitchrad);
	outlet(o.roll_x_rad, rollrad);

	var cy = Math.cos(yawrad * 0.5);
    var sy = Math.sin(yawrad * 0.5);
    var cp = Math.cos(pitchrad * 0.5);
    var sp = Math.sin(pitchrad * 0.5);
    var cr = Math.cos(rollrad * 0.5);
    var sr = Math.sin(rollrad * 0.5);
	var quat_w = (cr * cp * cy + sr * sp * sy);
    var quat_x = (sr * cp * cy - cr * sp * sy);
    var quat_y = (cr * sp * cy + sr * cp * sy);
    var quat_z = (cr * cp * sy - sr * sp * cy);
    outlet(o.quat_w, quat_w);
    outlet(o.quat_x, quat_x);
    outlet(o.quat_y, quat_y);
    outlet(o.quat_z, quat_z);
		
	// Do output
	
	var yawfloat = yaw + ensureFloat;
	var pitchfloat = pitch + ensureFloat;
	var rollfloat = roll + ensureFloat;
	
	if(flavour === "ambix_quaternion"){
		outlet(o.osc_msg, ["/quaternion", quat_w, -quat_x, quat_y, quat_z]);
	}
	if(flavour === "ambix_rotation_float"){
		outlet(o.osc_msg, ["/rotation", -pitchfloat, -yawfloat, -rollfloat]);
	}
	if(flavour === "ambix_rotation_int"){
		outlet(o.osc_msg, ["/rotation", -pitch, -yaw, -roll]);
	}
	if(flavour === "ambix_head_pose_float"){
		outlet(o.osc_msg, ["/head_pose", 0, ensureFloat, ensureFloat, ensureFloat, -pitchfloat, -yawfloat, -rollfloat]);
	}
	if(flavour === "ambix_head_pose_int"){
		outlet(o.osc_msg, ["/head_pose", 0, 0, 0, 0, -pitch, -yaw, -roll]);
	}
	if(flavour === "iem"){
		outlet(o.osc_msg, ["/SceneRotator/quaternions", quat_w, -quat_x, quat_y, quat_z]);
	}
	if(flavour === "unity"){
		outlet(o.osc_msg, ["/quaternions", quat_w, -quat_y, quat_z, -quat_x]);
	}
	if(flavour === "mach1"){
		outlet(o.osc_msg, ["/orientation", yawfloat, pitchfloat, rollfloat]);
	}
	if(flavour === "audiolab"){
		outlet(o.osc_msg, ["/rendering/htrpy", rollfloat, pitchfloat, yawfloat]);
	}
	if(flavour === "sparta_ypr"){
		outlet(o.osc_msg, ["/ypr", -yawfloat, -pitchfloat, rollfloat]);
	}
	if(flavour === "sparta_euler_individual"){
		outlet(o.osc_msg, ["/yaw", -yawfloat]);
		outlet(o.osc_msg, ["/pitch", -pitchfloat]);
		outlet(o.osc_msg, ["/roll", rollfloat]);
	}
	if(flavour === "sparta_quaternion"){
		outlet(o.osc_msg, ["/quaternion", quat_w, -quat_x, quat_y, quat_z]);
	}
	if(flavour === "3d_tune-in"){
		outlet(o.osc_msg, ["/3DTI-OSC/receiver/pry", deg2rad(pitchfloat), deg2rad(rollfloat), deg2rad(yawfloat)]);
	}
	if(flavour === "hedrot"){
		// (although pitch and roll seem swapped in my bino)
		outlet(o.osc_msg, ["/hedrot/yaw", yawfloat]);
		outlet(o.osc_msg, ["/hedrot/pitch", pitchfloat]);
		outlet(o.osc_msg, ["/hedrot/roll", rollfloat]);
	}
	
	// NX Bridge - always send so can be used in parallel
	//outlet(o.osc_msg, ["/bridge/quat", quat_w, quat_y, quat_x, -quat_z]);

}