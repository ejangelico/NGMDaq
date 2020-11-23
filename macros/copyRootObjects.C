void copyRootObjects(const char* sourcename, const char* targetname, const char* filter)
{
	TFile* sourcefile = TFile::Open(sourcename);
	if(!sourcefile)
	{
		std::cout<<"Unable to open "<<sourcename<<std::endl;
			return;
	}
	TFile* targetfile = TFile::Open(targetname,"UPDATE");
	if(!targetfile)
	{
		std::cout<<"Unable to open "<<targetname<<std::endl;
		return;
	}
	
	TIter next(sourcefile->GetListOfKeys());
	TObject* obj = 0;
	while((obj = next()))
	{
		if(TString(obj->GetName()).Contains(filter))
		{
			TObject* realObj = sourcefile->Get(obj->GetName());
			if(!realObj)
			{	
				std::cout<<"Error getting "<<obj->GetName()<<std::endl;
				continue;
			}
			targetfile->WriteTObject(realObj,realObj->GetName());
		}
	}
	targetfile->Close();
	sourcefile->Close();
}