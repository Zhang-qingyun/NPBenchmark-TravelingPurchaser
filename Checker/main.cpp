#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

//#include "Visualizer.h"

#include "../Solver/PbReader.h"
#include "../Solver/TravelingPurchase.pb.h"

using namespace std;
//using namespace szx;
using namespace pb;

int main(int argc, char *argv[]) {
	enum CheckerFlag {
		IoError = 0x0,
		FormatError = 0x1,
		DisconnectedError = 0x2,
		TotalValueError = 0x4
    };

    string inputPath;
    string outputPath;

    if (argc > 1) {
        inputPath = argv[1];
    } else {
        cout << "input path: " << flush;
        cin >> inputPath;
    }

    if (argc > 2) {
        outputPath = argv[2];
    } else {
        cout << "output path: " << flush;
        cin >> outputPath;
    }
	pb::TravelingPurchase::Input input;
    if (!load(inputPath, input)) { return ~CheckerFlag::IoError; }

    pb::TravelingPurchase::Output output;
    ifstream ifs(outputPath);
    if (!ifs.is_open()) { return ~CheckerFlag::IoError; }
    string submission;
    getline(ifs, submission); // skip the first line.
    ostringstream oss;
    oss << ifs.rdbuf();
    jsonToProtobuf(oss.str(), output);
	
    // check solution.
	int error = 0;
	
	for (auto temp = output.nodeidatmoment().begin(); temp != output.nodeidatmoment().end(); ++temp) {

		if (temp->nodeid() < 0 || temp->nodeid() >= input.nodeid().size()||temp->moment()>=input.periodlength()|| output.nodeidatmoment().begin()->nodeid()==input.sourcenode()|| output.nodeidatmoment().begin()->nodeid()==input.targetnode())
		{
			error |= CheckerFlag::FormatError;
		}
	}
	//check connectivity.
	int totalValue=0;
	for (auto temp = output.nodeidatmoment().begin(); temp != output.nodeidatmoment().end()-1; ++temp) {
		int flag = 0;
		for (auto in = input.edges().begin(); in != input.edges().end(); ++in) {
			if ((temp->nodeid() == in->source() && (temp + 1)->nodeid() == in->target())|| (temp->nodeid() == in->target() && (temp + 1)->nodeid() == in->source()))
				flag = 1;
		}
		if (flag == 0) {
			cout << "path " << temp->nodeid() << " to " << (temp + 1)->nodeid() << " is not connected!!" << endl;
			error |= CheckerFlag::DisconnectedError;
			
		}
		//check obj.
		vector<int> reFlag(input.noderequireds().size(),0) ;
		for (auto re = input.noderequireds().begin(); re != input.noderequireds().end(); ++re) {
			if (temp->nodeid() == re->nodeid()&& reFlag[re->id()] != 1)
			for (auto vam = re->valueatmoments().begin(); vam != re->valueatmoments().end(); ++vam) {
				if (temp->moment() == vam->moment()) {
					totalValue += vam ->value();
					reFlag[re->id()] = 1;//标记已被访问
					break;
				}
				else if (temp->moment() <= vam->moment()) {
					break;
				}
					
			}
		}
		
		//error |= CheckerFlag::TotalValueError;
		

	}
	cout << "the true totalvalue is " << totalValue << endl;

   

    //// visualize solution.
    //double pixelPerMinute = 1;
    //double pixelPerGate = 30;
    //int horizonLen = 0;
    //for (auto flight = input.flights().begin(); flight != input.flights().end(); ++flight) {
    //    horizonLen = max(horizonLen, flight->turnaround().end());
    //}

    //auto pos = outputPath.find_last_of('/');
    //string outputName = (pos == string::npos) ? outputPath : outputPath.substr(pos + 1);
    //Drawer draw;
    //draw.begin("Visualization/" + outputName + ".html", horizonLen * pixelPerMinute, input.airport().gates().size() * pixelPerGate, 1, 0);
    //f = 0;
    //for (auto gate = output.assignments().begin(); gate != output.assignments().end(); ++gate, ++f) {
    //    // check constraints.
    //    if ((*gate < 0) || (*gate >= input.airport().gates().size())) { continue; }
    //    bool incompat = false;
    //    for (auto ig = input.flights(f).incompatiblegates().begin(); ig != input.flights(f).incompatiblegates().end(); ++ig) {
    //        if (*gate == *ig) { incompat = true; break; }
    //    }
    //    const auto &flight(input.flights(f));
    //    draw.rect(flight.turnaround().begin() * pixelPerMinute, *gate * pixelPerGate, 
    //        (flight.turnaround().end() - flight.turnaround().begin()) * pixelPerMinute, pixelPerGate,
    //        false, to_string(f), "000000", incompat ? "00c00080" : "4080ff80");
    //}
    //draw.end();

    return (error == 0) ? totalValue : ~error;
}
