function reRenderAccident() {
    getRoad((res) => {
        res["Road"].sort(function (a, b) {
            if (a["NAME"] < b["NAME"]) return -1;
            if (a["NAME"] > b["NAME"]) return 1;
            return 0;
        });
        let accident_road_array = [];
        $("#selectAccidentNode").html(`<option value="">請選擇路口</option>`);
        $("#selectAccidentDate").html(`<option value="">請選擇日期</option>`);
        for (let i = 0; i < res["Road"].length; i++) {
            let road = res["Road"][i];
            let road_id = road["NAME"].split("k")[0] + "k"
            if (road["HOST"] != "unconnected" && accident_road_array.indexOf(road_id) == -1) {
                $("#selectAccidentNode").append(`<option value="${road_id}">${road_id}</option>`)
                accident_road_array.push(road_id)
            }
        }
        if (accident_node != "") {
            $("#selectAccidentNode").val(accident_node)
            selectAccidentNode(accident_node, true)
        }
    }, (error) => {
        console.log(error)
    })
}

function selectAccidentNode(node, reRenderFlag) {
    if (!reRenderFlag) {
        accident_date = "";
        accident_table.clear().draw();
        $("#selectAccidentDate").html(`<option value="">請選擇日期</option>`);
    }
    if (node == "") {
        return
    }
    getAccidentDateByNode(node, (res) => {
        accident_node = node;
        for (let i = 0; i < res["result"].length; i++) {
            $("#selectAccidentDate").append(`<option value="${res["result"][i]}">${res["result"][i]}</option>`)
        }
        if (accident_date != "") {
            $("#selectAccidentDate").val(accident_date)
            selectAccidentDate(accident_date)
        }
    }, (err) => {
        console.log(err)
    })
}

function selectAccidentDate(date) {
    if (accident_node == "") {
        return
    }
    getAccidentByDate(accident_node, date, (res) => {
        accident_table.clear().draw();
        accident_date = date;
        let data = res["result"];
        for (let i = 0; i < data.length; i++) {
            accident_table.row.add( [
                data[i][0],
                data[i][1],
                parseFloat(data[i][2]) / 30,
                data[i][3]
            ] ).draw(false);
        }
    }, (err) => {
        console.log(err)
    })
}